#pragma once

#include <sprout/string.hpp>

#include <bcl/string.hpp>
#include <bcl/double.hpp>

namespace bcl{
	namespace literals{
		namespace double_constant{
			template <char ... Chars>
			constexpr auto operator "" _dc()
			{
				constexpr auto str = ::sprout::make_string(Chars...);
				constexpr double d = ::bcl::stod(str);
				constexpr auto e = ::bcl::encode(d);

				return BCL_DOUBLE(e){};
			}
		}

		using double_constant::operator "" _dc;
	}
}

