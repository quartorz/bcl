#include <iostream>

#include <bcl/tuple.hpp>

#include <typeinfo>
#include <cxxabi.h>

template <typename T>
const char *name()
{
	return abi::__cxa_demangle(typeid(T).name(), 0, 0, nullptr);
}

int main()
{
	using t = bcl::tuple<int, float, long>;
	using s = bcl::tuple<double, short>;
	using u = bcl::tuple<char, wchar_t, short>;

	std::cout << name<bcl::tuple_merge_t<t, s, u>>() << std::endl;

	std::cout << name<bcl::tuple_cartesian_prod<
		bcl::tuple<bcl::tuple<int>, bcl::tuple<float>, bcl::tuple<long>>,
		bcl::tuple<bcl::tuple<short>, bcl::tuple<double>, bcl::tuple<char>>>::type>() << std::endl;

	std::cout << name<bcl::tuple_cartesian_prod_variadic<
		bcl::tuple<bcl::tuple<int>, bcl::tuple<float>, bcl::tuple<long>>,
		bcl::tuple<bcl::tuple<short>, bcl::tuple<double>, bcl::tuple<char>>>::type>() << std::endl;

	std::cout << name<bcl::tuple_cartesian_prod_variadic<
		bcl::tuple<bcl::tuple<int>, bcl::tuple<float>, bcl::tuple<long>>,
		bcl::tuple<bcl::tuple<short>, bcl::tuple<double>, bcl::tuple<char>>,
		bcl::tuple<bcl::tuple<unsigned>, bcl::tuple<unsigned char>>>::type>() << std::endl;
}

