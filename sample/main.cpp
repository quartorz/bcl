#include <bcl/tuple.hpp>
#include <bcl/value_tuple.hpp>

#include <iostream>
#include <typeinfo>

#include <cxxabi.h>

template <typename T>
const char *name()
{
        return abi::__cxa_demangle(typeid(T).name(), 0, 0, nullptr);
}

template <typename T>
const char *name(const T &)
{
        return abi::__cxa_demangle(typeid(T).name(), 0, 0, nullptr);
}

int main()
{
        bcl::tuple<int, float> a(0, 1.0f), b;

        bcl::get<0>(a) = 0;

        std::cout << name<bcl::tuple_element<1, bcl::tuple<int, float>>::type>() << std::endl;
        std::cout << bcl::tuple_find<long long, bcl::tuple<int, short, long, double, long long, float>>::type::value << std::endl;
        std::cout << bcl::value_tuple<int, 0, 29, 49, 671>::value<2> << std::endl;
        std::cout << bcl::value_tuple_find_t<int, 7, bcl::value_tuple<int, 9, 8, 7, 6>>::value << std::endl;
        std::cout << name<bcl::tuple_remove<bcl::value_tuple<int, 3>, bcl::tuple<int, float, double, short, char>>::type>() << std::endl;
        std::cout << name<bcl::tuple_common_element_t<bcl::tuple<char, short, int, long>, bcl::tuple<short, float, double, long>>>() << std::endl;
        std::cout << name<bcl::tuple_element_subtract_t<bcl::tuple<char, short, int, long>, bcl::tuple<short, float, double, long>>>();
}

