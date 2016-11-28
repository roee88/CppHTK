#pragma once
#include <vector>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Generates a set of triangular filter weights using the code present in HTK. For more
/// information, please refer to the notebook explaining all this in more detail.
/// </summary>
///
/// <param name="numChans">  	Number of channels. </param>
/// <param name="frameSize"> 	(Optional) frame size. </param>
/// <param name="sampPeriod">	(Optional) the sample period. </param>
/// <param name="lopass">	 	(Optional) the lopass frequency. </param>
/// <param name="hipass">	 	(Optional) the hipass frequency. </param>
/// <param name="silent">	 	(Optional) true to silent output to std::cout. </param>
///
/// <returns>	The filter as vector of tuples. </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::tuple<float, short>> gen_filter(const int numChans, 
	int frameSize = 400,
	long sampPeriod = 625,
	float lopass = 80,
	float hipass = 7500,
	bool silent = false);
