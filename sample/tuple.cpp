#include <iostream>

#include <bcl/tuple.hpp>
#include <bcl/tuple_operation.hpp>

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
}

