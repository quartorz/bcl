#pragma once

namespace bcl{
	template <typename T>
	constexpr T sqrt(T s)
	{
		T x = s / 2.0;
		T prev = 0.0;

		while(x != prev){
			prev = x;
			x = (x + s / x) / 2.0;
		}

		return x;
	}
}

