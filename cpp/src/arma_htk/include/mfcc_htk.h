////////////////////////////////////////////////////////////////////////////////////////////////////
// file:	mfcc_htk.h
//
// summary:	Declares the CHTKExtractor class -> This is the one you should generally use.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <iostream>
#include <memory>
#include <armadillo>

using namespace std::string_literals;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	Class to compute HTK compatible MFCC features from audio. </summary>
/// <details>
/// It is designed to be as close as possible to the HTK implementation.
/// For details on HTK implementationlook for HTK Book online, specifically
/// chapter 5 titled "Speech Input/Output".
///
/// This implementation was somewhat based upon the HTK source code.Most
/// of the interesting code can be found in ... / HTKLib / HSigP.c file.
///
/// The latest version of HTK available during writing of this class was 3.4.1.
///
/// HTK is licensed by University of Cambridge and isn't in any way related
/// to this particular Python implementation (so don't bother them if you
///	have problems with this code).
///
///	For more information about HTK go to http ://htk.eng.cam.ac.uk/
/// </details>
////////////////////////////////////////////////////////////////////////////////////////////////////

class MFCC_HTK
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	HTK configuration </summary>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	struct Config {

		/// <summary>
		/// filter_compatibility (boolean):
		/// 	load the filter specification similar to HTK. This exists to allow binary comaptibility with
		/// 	HTK, because they implement the filters slightly differently than mentioned in their docs.
		/// 
		/// If you set filter_compatibility to false, a built-in method will be used to create half -
		///		overlapping triangular filters spread evenly between lo_freq and hi_freq in the mel domain.
		/// </summary>
		bool filter_compatibility = false;

		/// <summary>
		/// win_len(int) : Length of frame in samples. Default value is 400, which is
		///		equal to 25 ms for a signal sampled at 16 kHz (i.e. 2.5x the win_shift length)
		/// Equivalent: WINDOWSIZE divided by SOURCERATE.
		/// </summary>
		int win_len = 400;

		/// <summary>
		/// win_shift(int) : Frame shift in samples - in other words, distance between
		///		the start of two consecutive frames. Default value is 160, which is equal to 10 ms for a
		///		signal sampled at 16 kHz. This is generates 100 frames per second of the audio, which is a
		///		standard framerate for many audio tasks. 
		/// Equivalent: TARGETRATE divided by SOURCERATE.
		/// </summary>
		int win_shift = 160;

		/// <summary>
		/// preemph(float) : Preemphasis coefficient.This is used to calculate 
		///		first-order difference of the signal. 
		/// Equivalent: PREEMCOEF.
		/// </summary>
		float preemph = 0.97f;

		/// <summary>
		/// filter_num(int) : Number of triangular filters used to reduce the spectrum. 
		///		Default value is 26. 
		/// Equivalent: NUMCHANS.
		/// </summary>
		int filter_num = 26;


		/// <summary>
		/// lifter_num(int) : Default value is 22.
		/// Equivalent: CEPLIFTER.
		/// </summary>
		int lifter_num = 22;

		/// <summary>
		/// mfcc_num(int) : Number of MFCCs computed.Default value is 12.
		/// Equivalent: NUMCEPS.
		/// </summary>
		int mfcc_num = 12;

		/// <summary>
		/// lo_freq(float) : Lowest frequency(in Hz) used in computation. Default value is
		///		0 Hz. This is used exclusively to compute filters. 
		/// Equivalent: LOFREQ.
		/// </summary>
		int lo_freq = -1;

		/// <summary>
		/// hi_freq(float) : Highest frequency(in Hz) used in computation. 
		///		Default value is the Nyquist frequency. This is used exclusively 
		///		to compute filters. 
		/// Equivalent: HIFREQ.
		/// </summary>
		int hi_freq = -1;

		/// <summary>
		/// samp_freq(int) : Sampling frequency of the audio. Default value
		///		is 16000, which is a common value for recording speech. Due to 
		///		Nyquist, the maximum frequency stored is half of this value, i.e. 8000 Hz.
		///	Equivalent: 10^7 / SOURCERATE
		/// </summary>
		int samp_freq = 16000;

		/// <summary> raw_energy(boolean) : Should the energy be computed from
		///		the raw signal, or (if false) should the 0'th coepstral coefficient
		///		be used instead, which is almost equivalent and much faster to 
		///		compute (since we compute MFCC anyway).
		///	Equivalent: RAWENERGY
		/// </summary>
		bool raw_energy = false;
		
        /// <summary>
        /// feat_melspec(boolean) : Should the spectral features be added to 
        ///		output. These are the values of the logarithm of the filter outputs.
        ///		The number of these features is eqeual to filter_num.
        /// </summary>
        bool feat_melspec = false;

		/// <summary>	
		/// feat_mfcc(boolean) : Should MFCCs be added to the output.
		///		The number of these features is equal to mfcc_num.
		///	</summary>
		bool feat_mfcc = true;

		/// <summary>
		/// feat_energy(boolean) : Should energy be added to the output.
		///		This is a single value.
		/// </summary>
		bool feat_energy = true;
        
		/// <summary>
		/// ceps_energy (boolean): Energy is calculated from the 0th cepstral 
		///		coefficient. Default is true.
		///	Equivalent: true equals to option _0 in HTK; false equals to _E.
		/// </summary>
		bool ceps_energy = true;

		/// <summary>	enormalise (boolean): subtract max value of energy 
		///		and add 1.0. Only applied to normal (non cepstral) energy. 
		///		(default false). 
		///	Equivalent: ENORMALISE
		/// </summary>
		bool enormalise = false;

		/// <summary> 
		/// sil_floor (float, dB): The lowest energy in the utterance can be 
		///		clamped using this configuration parameter which gives the ratio 
		///		between the maximum and minimum energies in the utterance in dB. 
		///		(default 50). 
		/// </summary>
		float sil_floor = 50.0f;

		/// <summary> 
		/// escale (float): scale energy by this value. 
		///		only applied to normal (non cepstral) energy. 
		///	Equivalent: ESCALE
		/// </summary>
		float escale = 1.0f;

		/// <summary> perform Cepstral Mean Normalization - 
		///		subtract utterance mean from cepstral coefficients only. 
		/// Equivalent: _Z in HTK options. (default false). 
		/// </summary>
		bool cmn = false;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Constructor. </summary>
	///
	/// <param name="config">	The configuration. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	MFCC_HTK(const Config& config);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	
	/// Helper method that loads a 16-bit signed int RAW signal from file.
	/// Uses system - natural endianess(most likely little - endian).
	/// If you have a problem with endianess use "byteswap()" method on resulting array.
	/// </summary>
	///
	/// <param name="filename">	Filename of the raw signal file. </param>
	///
	/// <returns>	vector of raw signal. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	arma::vec load_raw_signal(std::string filename);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	
	/// Gets the features from an audio signal based on the configuration set in constructor. 
	///	</summary>
	///
	/// <param name="signal">	The audio signal. </param>
	///
	/// <returns>	
	/// A WxF matrix, where W is the number of windows in the signal and F is the number of chosen 
	/// features.
	///	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	arma::mat get_feats(arma::vec signal);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Computes delta using the HTK method. </summary>
	///
	/// <param name="feat"> Matrix of shape WxF, where W is number of frames
	///	and F is number of features.. </param>
	/// <param name="deltawin">	(Optional) the DELTAWINDOW parameter of the delta computation.
	///	Check HTK Book Chapter 5.6 for details.. </param>
	/// 
	/// <returns>	A matrix of the same size as argument feat containing the deltas of the provided 
	/// 			features. 
	/// </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	arma::mat get_delta(arma::mat feat, int deltawin = 2);

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	
	/// Creates filter spec to reproduce an HTK bug.
	/// Normally, create_filter should be used instead.
	/// </summary>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void create_filter_htk();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Creates filter specified by their count. </summary>
	///
	/// <param name="num">	Number of filters. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void create_filter();


	/// <summary>	The configuration. </summary>
	Config config_;
	
	/// <summary>	Length of the FFT. </summary>
	int fft_len_;
	
	/// <summary>	The filter matrix. </summary>
	arma::mat filter_mat_;

	/// <summary>	The hamming vector. </summary>
	arma::vec hamm_;

	/// <summary>	The dct base. </summary>
	arma::mat dct_base_;

	/// <summary>	The lifter vector. </summary>
	arma::vec lifter_;

	/// <summary>	The mfnorm. </summary>
	double mfnorm_;
};
