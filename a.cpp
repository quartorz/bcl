#include <bcl/string/stod.hpp>

#include <kv/conv-double.hpp>

int main()
{
	std::cout.precision(16);
	std::cout << bcl::stod("1.79769313486231571e+308", nullptr, 1) << std::endl;
	std::cout << bcl::stod("1.79769313486231571e+308", nullptr, -1) << std::endl;
	std::cout << bcl::stod("4.9406564584124654e-324", nullptr, 1) << std::endl;
	std::cout << bcl::stod("4.9406564584124654e-324", nullptr, -1) << std::endl;
	std::cout << bcl::stod("3.9406564584124654e-324", nullptr, 1) << std::endl;
	std::cout << bcl::stod("3.9406564584124654e-324", nullptr, -1) << std::endl;
	std::cout << bcl::stod("5.9406564584124654e-324", nullptr, 1) << std::endl;
	std::cout << bcl::stod("5.9406564584124654e-324", nullptr, -1) << std::endl;
	std::cout << bcl::stod("1.2345e-1", nullptr) << std::endl;

	constexpr auto x = bcl::stod("1.2345e1");

	std::cout << x << std::endl;

	std::cout << bcl::stod("3.0") << std::endl;

	std::cout << kv::conv_double::stringtod("1.79769313486231571e+308", 1) << std::endl;
	std::cout << kv::conv_double::stringtod("1.79769313486231571e+308", -1) << std::endl;
	std::cout << kv::conv_double::stringtod("4.9406564584124654e-324", 1) << std::endl;
	std::cout << kv::conv_double::stringtod("4.9406564584124654e-324", -1) << std::endl;
	std::cout << kv::conv_double::stringtod("3.9406564584124654e-324", 1) << std::endl;
	std::cout << kv::conv_double::stringtod("3.9406564584124654e-324", -1) << std::endl;
	std::cout << kv::conv_double::stringtod("5.9406564584124654e-324", 1) << std::endl;
	std::cout << kv::conv_double::stringtod("5.9406564584124654e-324", -1) << std::endl;
}

