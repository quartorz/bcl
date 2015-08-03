#pragma once

#include <utility>
#include <type_traits>

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

// check if std::declval<T>().name(std::declval<U>()...) is well-defined
#define BCL_HAS_FUNCTION(name)\
	namespace detail{\
		template <typename T, typename ... U>\
		auto test_ ## name ## _func(int)\
			-> decltype(::std::declval<T>().name(::std::declval<U>()...), void(0), ::std::true_type{});\
		template <typename T, typename ... U>\
		::std::false_type test_ ## name ## _func(long);\
	}\
	template <typename T, typename ... Args>\
	class has_ ## name ## _func : decltype(detail::test_ ## name ## _func<T, Args...>()){\
	};\

