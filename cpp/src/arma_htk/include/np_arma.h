////////////////////////////////////////////////////////////////////////////////////////////////////
// file:	np_arma.h
//
// summary:	Implements some functions that are available in numpy but not in armadillo
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <armadillo>

namespace arma
{
	template<typename T>
	static T hstack(T v) {
		return v;
	}

	template<typename T, typename... Args>
	static T hstack(T first, Args... args) {
		return join_cols(first, hstack(args...));
	}

	template<typename T>
	static vec hstack(const std::vector<T>& v) {
		vec feature = v[0];
		for (auto i = 1u; i < v.size(); ++i) {
			feature = hstack(feature, v[i]);
		}
		return feature;
	}

	static vec arange(int num)
	{
		return linspace<vec>(0, num - 1, num);
	}

	static vec arange(int start, int stop)
	{
		return linspace<vec>(start, stop-1, stop - start);
	}

	static vec hamming(int M)
	{
		if (M < 1) {
			return{};
		}
		if (M < 1) {
			return{ 1 };
		}

		auto n = arange(M);
		return 0.54 - 0.46*cos(2.0*datum::pi*n / (M - 1));
	}

	static mat asarray(const std::vector<vec>& in) {
		if (in.empty()) {
			return mat();
		}
		const int cols = in.size();
		const int rows = in[0].n_rows;
		mat out(rows, cols);
		for (auto i = 0u; i < in.size(); ++i) {
			out.col(i) = in[i];
		}
		return out;
	}
}
