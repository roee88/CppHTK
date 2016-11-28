#include "htk_file.h"
#include <fstream>
#include <iostream>

int32_t read32(std::ifstream& f) {
	int32_t val = 0;
	f.read(reinterpret_cast<char*>(&val), sizeof(int32_t));
#ifdef _MSC_VER
	return _byteswap_ulong(val);
#else
	return __builtin_bswap32(val);
#endif
}

int16_t read16(std::ifstream& f) {
	int16_t val = 0;
	f.read(reinterpret_cast<char*>(&val), sizeof(int16_t));
#ifdef _MSC_VER
	return _byteswap_ushort(val);
#else
	return __builtin_bswap16(val);
#endif
}

float readfloat(std::ifstream& f) {
	int32_t val = read32(f);
	return *reinterpret_cast<float*>(&val);
}

bool HTKFile::load(const std::string & filename)
{
	std::ifstream f;
	f.open(filename, std::ios::in | std::ios::binary);
	if (!f) {
		return false;
	}

	uint16_t sampSize;
	uint16_t paramKind;

	nSamples_ = read32(f);
	sampPeriod_ = read32(f);
	sampSize = read16(f);
	paramKind = read16(f);

	std::string kinds[] = {"WAVEFORM", "LPC", "LPREFC", "LPCEPSTRA", "LPDELCEP", "IREFC", 
		"MFCC", "FBANK", "MELSPEC", "USER", "DISCRETE", "PLP", "ERROR"};
	basicKind_ = kinds[paramKind & 0x3F];

	if ((paramKind & 0100) != 0)
		qualifiers_.insert("E");
	if ((paramKind & 0200) != 0)
		qualifiers_.insert("N");
	if ((paramKind & 0400) != 0)
		qualifiers_.insert("D");
	if ((paramKind & 01000) != 0)
		qualifiers_.insert("A");
	if ((paramKind & 02000) != 0)
		qualifiers_.insert("C");
	if ((paramKind & 04000) != 0)
		qualifiers_.insert("Z");
	if ((paramKind & 010000) != 0)
		qualifiers_.insert("K");
	if ((paramKind & 020000) != 0)
		qualifiers_.insert("0");
	if ((paramKind & 040000) != 0)
		qualifiers_.insert("V");
	if ((paramKind & 0100000) != 0)
		qualifiers_.insert("T");

	nFeatures_ = sampSize;
	if (qualifiers_.find("C") != qualifiers_.end() ||
		qualifiers_.find("V") != qualifiers_.end() ||
		basicKind_ == "IREFC" || basicKind_ == "WAVEFORM") {
		nFeatures_ = sampSize / 2;
	}
	else {
		nFeatures_ = sampSize / 4;
	}

	if (qualifiers_.find("C") != qualifiers_.end()) {
		nSamples_ -= 4;
	}

	if (qualifiers_.find("V") != qualifiers_.end()) {
		throw std::runtime_error("VQ is not implemented");
	}
	

	data_.reshape(nSamples_, nFeatures_);

	if (basicKind_ == "IREFC" || basicKind_ == "WAVEFORM") {
		for (int x = 0; x < nSamples_; ++x) {
			for (int v = 0; v < nFeatures_; ++v) {
				int16_t val = read16(f);
				data_(x, v) = val / 32767.0;
			}
		}
	}
	else if (qualifiers_.find("C") != qualifiers_.end()) {
		std::vector<float> A(nFeatures_, 0.0f);
		for (int x = 0; x < nFeatures_; ++x) {
			float v = readfloat(f);
			A.push_back(v);
		}
		std::vector<float> B(nFeatures_, 0.0f);
		for (int x = 0; x < nFeatures_; ++x) {
			float v = readfloat(f);
			B.push_back(v);
		}

		for (int x = 0; x < nSamples_; ++x) {
			for (int v = 0; v < nFeatures_; ++v) {
				int16_t val = read16(f);
				data_(x, v) = val + B[v] / A[v];
			}
		}
	}
	else {
		for (int x = 0; x < nSamples_; ++x) {
			for (int v = 0; v < nFeatures_; ++v) {
				float val = readfloat(f);
				data_(x, v) = val;
				if (f.eof()) {
					throw std::runtime_error("unexpected end of file");
				}
			}
		}
	}

	if (qualifiers_.find("K") != qualifiers_.end()) {
		std::cout << "CRC checking not implememnted...";
	}

	return true;
}
