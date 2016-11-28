#include "mfcc_htk.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "np_arma.h"
#include "gen_filt.h"

MFCC_HTK::MFCC_HTK(const Config & config)
	: config_(config)
{
	fft_len_ = static_cast<int>(pow(2, floor(log2(config_.win_len)) + 1));

	// Fix out of range lo/hi freq
	config_.lo_freq = std::max(0, config_.lo_freq);
	if (config_.hi_freq < 0)
		config_.hi_freq = config_.samp_freq / 2;

	// This uses HTK code to reproduce a bug in HTK in filter map generation
	if (config_.filter_compatibility) {
		create_filter_htk();
	}
	// This should normally be used
	else {
		create_filter();
	}

	hamm_ = arma::hamming(config_.win_len);

	dct_base_ = arma::zeros<arma::mat>(config_.filter_num, config_.mfcc_num);
	for (int m = 0; m < config_.mfcc_num; ++m) {
		dct_base_.col(m) = arma::cos((m + 1)*arma::datum::pi / config_.filter_num*(arma::arange(config_.filter_num) + 0.5));
	}

	lifter_ = 1 + (config_.lifter_num / 2)*arma::sin(arma::datum::pi*(1 + arma::arange(config_.mfcc_num)) / config_.lifter_num);

	mfnorm_ = sqrt(2.0 / config_.filter_num);
}

arma::vec MFCC_HTK::load_raw_signal(std::string filename)
{
	std::ifstream file;
	file.open(filename, std::ios_base::in | std::ios_base::binary);
	if(file.is_open())
	{
		arma::Col<arma::s16> content;
		if (content.quiet_load(file, arma::raw_binary))
		{
			return arma::conv_to<arma::vec>::from(content);
		}
	}
	return arma::vec();
}

arma::mat MFCC_HTK::get_feats(arma::vec signal)
{
	auto sig_len = signal.size();
	auto win_num = (sig_len - config_.win_len) / config_.win_shift + 1;
	
	std::vector<arma::vec> feats;

	for (auto w = 0u; w < win_num; ++w) {
		std::vector<arma::vec> featwin;

		// extract window
		auto s = w * config_.win_shift;
		auto e = s + config_.win_len;
		arma::vec win = signal.rows (s, e - 1);

		// raw energy is calculated before any windowing or pre-emphasis
		double energy;
		if (config_.feat_energy && !config_.ceps_energy && config_.raw_energy) {
			energy = ::log(arma::sum(arma::square(win)));
		}

		// preemphasis
		win -= arma::hstack(arma::vec{ win[0] }, win.head_rows(win.n_rows - 1)) 
			* config_.preemph;

		// windowing
		win %= hamm_;

		// for energy calculations
		arma::vec sig_win = win;

		// fft
		win = arma::abs(arma::fft(win, fft_len_));
		win = win.head_rows(filter_mat_.n_rows);

		// filters
		arma::vec melspec = (win.t() * filter_mat_).t();

		// floor (before log)
		melspec(arma::find(melspec < 0.001)).fill(0.001);

		// log
		melspec = arma::log(melspec);

		if (config_.feat_melspec) {
			featwin.push_back(melspec);
		}

		// dct
		arma::vec mfcc = (melspec.t() * dct_base_).t();
		mfcc *= mfnorm_;

		// lifter
		mfcc %= lifter_;

		// sane fixes
		mfcc(arma::find_nonfinite(mfcc)).fill(0);

		if (config_.feat_mfcc) {
			featwin.push_back(mfcc);
		}

		// energy
		if (config_.feat_energy) {
			if (config_.ceps_energy) {
				energy = arma::sum(melspec) * mfnorm_;
			}
			else if (!config_.raw_energy) {
				energy = ::log(arma::sum(arma::square(sig_win)));
			}

			// sane fixes
			if (arma::arma_isnan(energy) || arma::arma_isinf(energy)) {
				energy = 0;
			}

			featwin.push_back(arma::vec{ energy });
		}
		
		feats.push_back(arma::hstack(featwin));
	}

	auto ret = arma::asarray(feats);

	if (config_.cmn) {
		int with_ceps_energy = (config_.ceps_energy) ? 0 : -1;
		arma::mat mean = arma::mean(ret(arma::span(0, config_.mfcc_num + with_ceps_energy), arma::span::all), 1);
		for (int i = 0; i < ret.n_cols; ++i) {
			ret(arma::span(0, config_.mfcc_num + with_ceps_energy),i) -= mean;
		}
	}

	if (config_.feat_energy && config_.enormalise && !config_.ceps_energy) {
		auto max = arma::max(ret(config_.mfcc_num, arma::span::all));
		auto min = max - (config_.sil_floor * ::log(10.0)) / 10.0;
		ret(config_.mfcc_num, arma::span::all) = arma::clamp(ret(config_.mfcc_num, arma::span::all), min, max);
		ret(config_.mfcc_num, arma::span::all) = 1.0 - (max - ret(config_.mfcc_num, arma::span::all)) * config_.escale;
	}

	return ret;
}

arma::mat MFCC_HTK::get_delta(arma::mat feat, int deltawin)
{
	std::vector<arma::vec> deltas;

	double norm = 2.0*arma::sum(arma::square(arma::arange(1, deltawin + 1)));
	auto win_num = feat.n_cols;
	auto win_len = feat.n_rows;

	for (auto win = 0u; win < win_num; ++win)
	{
		arma::vec delta = arma::zeros(win_len);

		for (int t = 1; t < deltawin + 1; ++t)
		{
			int tm = win - t;
			if (tm < 0)
				tm = 0;

			int tp = win + t;
			if (tp >= win_num)
				tp = win_num - 1;

			delta += (t*(feat.col(tp) - feat.col(tm))) / norm;
		}

		deltas.push_back(delta);		
	}

	return arma::asarray(deltas);
}

void MFCC_HTK::create_filter_htk()
{
	long sampPeriod = 10000000 / config_.samp_freq;
	auto reader = gen_filter(config_.filter_num, config_.win_len, sampPeriod,
		config_.lo_freq, config_.hi_freq, true);
	int filter_num = 0;
	for (auto i = 0u; i < reader.size(); ++i) {
		filter_num = std::max(filter_num, (int)std::get<1>(reader[i]));
	}

	config_.filter_num = filter_num;
	filter_mat_ = arma::zeros(fft_len_ / 2, config_.filter_num);
	for (auto i = 0u; i < reader.size(); ++i) {
		auto wt = std::get<0>(reader[i]);
		auto bin = static_cast<int>(std::get<1>(reader[i]));

		if (bin < 0)
			continue;
		if (bin > 0)
			filter_mat_(i, bin - 1) = wt;
		if (bin < config_.filter_num)
			filter_mat_(i, bin) = 1 - wt;
	}
}

void MFCC_HTK::create_filter()
{
	filter_mat_ = arma::zeros(fft_len_ / 2, config_.filter_num);

	auto mel2freq = [](arma::vec mel) {
		return arma::vec{ 700.0*(arma::exp((mel) / 1127.0) - 1) };
	};

	auto freq2mel = [](double freq) {
		return 1127 * (::log(1 + ((freq) / 700.0)));
	};

	double lo_mel = freq2mel(config_.lo_freq);
	double hi_mel = freq2mel(config_.hi_freq);

	auto mel_c = arma::linspace(lo_mel, hi_mel, config_.filter_num + 2);
	auto freq_c = mel2freq(mel_c);

	arma::vec point_c_1 = freq_c/(float)config_.samp_freq * fft_len_;
	auto point_c = arma::conv_to<arma::ivec>::from(point_c_1);

	for (int f = 0; f < config_.filter_num; ++f) {
		auto d1 = point_c[f + 1] - point_c[f];
		auto d2 = point_c[f + 2] - point_c[f + 1];
		
		filter_mat_(arma::span(point_c[f], point_c[f+1]), f) = 
			arma::linspace(0, 1, d1 + 1);
		filter_mat_(arma::span(point_c[f+1], point_c[f + 2]), f) = 
			arma::linspace(1, 0, d2 + 1);
	}
}
