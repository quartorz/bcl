#pragma once

#include <utility>

namespace bcl{
	namespace detail{
		template <typename T, decltype(&T::value) = &T::value>
		::std::true_type test_value(int);
		template <typename T>
		::std::false_type test_value(long);
	}

	template <typename T>
	struct has_value : decltype(detail::test_value<T>(0))
	{
	};

	template <typename T>
	constexpr auto has_value_v = has_value<T>::value;
}

