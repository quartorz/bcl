#pragma once

namespace bcl{
	constexpr double pow2(int n)
	{
		double x = 1.0;

		if(n > 0){
			while(n-- != 0)
				x *= 2.0;
		}else if(n < 0){
			while(n++ != 0)
				x /= 2.0;
		}

		return x;
	}
}

