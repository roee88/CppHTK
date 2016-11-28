////////////////////////////////////////////////////////////////////////////////////////////////////
// file:	htk_file.h
//
// summary:	Declares the CHTKFile class for reading HTK formatted files
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <cstdint>
#include <set>
#include <vector>
#include <armadillo>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary> Class to load binary HTK file.
/// Details on the format can be found online in HTK Book chapter 5.7.1.
/// Not everything is implemented 100 % , but most features should be supported.
/// Not implemented:
/// CRC checking - files can have CRC, but it won't be checked for correctness.
/// VQ - Vector features are not implemented.
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////

class HTKFile
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Loads from given file. </summary>
	///
	/// <param name="filename">	The filename of the HTK file to load. </param>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	bool load(const std::string& filename);

	auto data() const { return data_.t(); }
	
	int32_t samples() const { return nSamples_; }

	int32_t features() const { return nFeatures_; }

	int32_t samp_period() const { return sampPeriod_; }

	const auto& basic_kind() const { return basicKind_; }

	const auto& qualifiers() const { return qualifiers_; }

private:
	arma::mat data_;
	int32_t nSamples_;
	int32_t nFeatures_;
	int32_t sampPeriod_;
	std::string basicKind_;
	std::set<std::string> qualifiers_;
};

