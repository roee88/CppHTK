#include <iostream>
#include "np_arma.h"
#include "mfcc_htk.h"
#include "htk_file.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	
///	Main entry-point for this test application.
///	Shows usage of MFCC_HTK and HTKFile.
///	Compares the output of the hcopy program (as read by HTKFile) to the output of MFCC_HTK.
///	See code for details.
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	cout << "Armadillo version: " << arma::arma_version::as_string() << endl;
	
	// configuration
	MFCC_HTK::Config config;
	config.filter_compatibility = true;	// filter map HTK compatibility
	config.lo_freq = 80;
	config.hi_freq = 7500;

	// setting up the main class
	MFCC_HTK mfcc{ config };

	// here we load the raw audio file
	auto sig = mfcc.load_raw_signal("./example/file.raw");

	// DC removal
//	sig = sig - arma::mean(sig);

	// here we calculate the MFCC + energy, deltas and acceleration coefficients
	auto feat = mfcc.get_feats(sig);
	auto delta = mfcc.get_delta(feat, 2);
	auto acc = mfcc.get_delta(delta, 2);

	// here we merge the MFCCs and deltas together to get 39 features
	feat = arma::hstack(feat, delta, acc);

	// here we use HTK to calculate the same thing
	// you can comment this line if you don't have HTK installed
	std::system("hcopy -C ./example/hcopy.conf -T 1 ./example/file.raw ./example/file.htk"); 

	// here we load the features generate by the command above
	auto htk = HTKFile();
	htk.load("./example/file.htk");

	// calculating the difference between features
	arma::mat diff = feat - htk.data();

	// computing and dsiplaying the maximum difference between the two methods
	std::cout << "Maximum difference: " 
		<< arma::max(arma::max(arma::abs(diff))) << std::endl;

	// do not close terminal immediately
	std::getchar();
	return 0;
}