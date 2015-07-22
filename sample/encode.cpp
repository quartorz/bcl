#include <bcl/double.hpp>

#include <sprout/math/signbit.hpp>
#include <sprout/math/float_significand.hpp>
#include <sprout/math/float2_significand.hpp>
#include <sprout/math/float2_exponent.hpp>
#include <sprout/math/pow.hpp>
#include <iostream>
#include <iomanip>

int main()
{
	std::cout.setf(std::ios::scientific);
	std::cout.precision(16);

	constexpr auto enc1 = bcl::encode(0.1234567890123456);
	using enc1_t = bcl::get_encoded_type_t<std::get<0>(enc1), std::get<1>(enc1), std::get<2>(enc1), std::get<3>(enc1)>;

	constexpr auto enc2 = bcl::encode(std::numeric_limits<double>::infinity());
	using enc2_t = bcl::get_encoded_type_t<std::get<0>(enc2), std::get<1>(enc2), std::get<2>(enc2), std::get<3>(enc2)>;

	std::cout << enc1_t::value << std::endl;
	std::cout << enc2_t::value << std::endl;
	std::cout << D_T(0.1234567890123456)::value << std::endl;

	constexpr auto v = D_(0.1234567890123456);
	constexpr auto w = D_(0.01234567890123456);

	std::cout << decltype(v + w)::value << std::endl;
	std::cout << v + w << std::endl;

	std::cout << std::numeric_limits<double>::max() << std::endl;
	std::cout << D_T(std::numeric_limits<double>::max())::value << std::endl;

	std::cout << std::numeric_limits<double>::denorm_min() << std::endl;
	std::cout << D_T(std::numeric_limits<double>::denorm_min())::value << std::endl;
}

