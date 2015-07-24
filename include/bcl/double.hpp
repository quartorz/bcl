#pragma once

#include <cstdint>
#include <limits>
#include <tuple>
#include <type_traits>

#include <sprout/math/isnan.hpp>
#include <sprout/math/signbit.hpp>
#include <sprout/math/float2_exponent.hpp>
#include <sprout/math/isnormal.hpp>
#include <sprout/math/fabs.hpp>

#include <bcl/pow2.hpp>

namespace bcl{
	namespace detail{
		template <bool Negative, ::std::uint64_t Significand, int Exponent>
		struct double_{
			using value_type = double;

			static constexpr double value =
				(Negative ? -1.0 : 1.0) * static_cast<double>(Significand) * ::bcl::pow2(Exponent);

			constexpr operator double() const
			{
				return value;
			}
		};

		template <bool Negative>
		struct zero{
			using value_type = double;

			static constexpr double value = (Negative ? -0.0 : 0.0);

			constexpr operator double() const
			{
				return value;
			}
		};

		template <bool Negative>
		struct inf{
			using value_type = double;

			static constexpr double value = Negative
				   ? -::std::numeric_limits<double>::infinity()
				   : ::std::numeric_limits<double>::infinity();

			constexpr operator double() const
			{
				return value;
			}
		};

		struct qnan{
			using value_type = double;

			static constexpr double value = ::std::numeric_limits<double>::quiet_NaN();

			constexpr operator double() const
			{
				return value;
			}
		};

		struct snan{
			using value_type = double;

			static constexpr double value = ::std::numeric_limits<double>::signaling_NaN();

			constexpr operator double() const
			{
				return value;
			}
		};

		enum class encode_type{
			number, zero, inf, qnan, snan
		};

		constexpr ::std::tuple<::std::uint64_t, int> encode_impl(double x)
		{
			x = ::sprout::fabs(x);

			if(x < 1.0){
				int i = 0;

				while(::std::int64_t(x) != x){
					x *= 2.0;
					i--;
				}

				return ::std::make_tuple(::std::uint64_t(x), i);
			}else{
				int i = 0;

				while(x > 1.0){
					x /= 2.0;
					i++;
				}

				return ::std::make_tuple(::std::uint64_t(x * ::bcl::pow2(53)), i - 53);
			}
		}

		constexpr ::std::tuple<encode_type, bool, ::std::uint64_t, int>
		encode(double x)
		{
			if(x == 0.0)
				return ::std::make_tuple(encode_type::zero, ::sprout::signbit(x), 0, 0);
			else if(x == inf<false>::value)
				return ::std::make_tuple(encode_type::inf, false, 0, 0);
			else if(x == inf<true>::value)
				return ::std::make_tuple(encode_type::inf, true, 0, 0);
			else if(::sprout::isnan(x))
				// TODO: distinguish between quiet NaN and signaling NaN
				return ::std::make_tuple(encode_type::qnan, false, 0, 0);
			else{
				auto e = encode_impl(x);
				return ::std::make_tuple(
					encode_type::number, x < 0.0,
					::std::get<0>(e),
					::std::get<1>(e));
			}
		}
	}

	using detail::encode;

	template <detail::encode_type T, bool Negative, ::std::uint64_t Significand, int Exponent>
	struct get_encoded_type{
		using type = detail::double_<Negative, Significand, Exponent>;
	};

	template <bool Negative, ::std::uint64_t Significand, int Exponent>
	struct get_encoded_type<detail::encode_type::zero, Negative, Significand, Exponent>{
		using type = detail::zero<Negative>;
	};

	template <bool Negative, ::std::uint64_t Significand, int Exponent>
	struct get_encoded_type<detail::encode_type::inf, Negative, Significand, Exponent>{
		using type = detail::inf<Negative>;
	};

	template <bool Negative, ::std::uint64_t Significand, int Exponent>
	struct get_encoded_type<detail::encode_type::qnan, Negative, Significand, Exponent>{
		using type = detail::qnan;
	};

	template <bool Negative, ::std::uint64_t Significand, int Exponent>
	struct get_encoded_type<detail::encode_type::snan, Negative, Significand, Exponent>{
		using type = detail::snan;
	};

	template <detail::encode_type T, bool Negative, ::std::uint64_t Significand, int Exponent>
	using get_encoded_type_t = typename get_encoded_type<T, Negative, Significand, Exponent>::type;
}

#define BCL_DOUBLE(x)\
	::bcl::get_encoded_type_t<\
		::std::get<0>(x),\
		::std::get<1>(x),\
		::std::get<2>(x),\
		::std::get<3>(x)>

#define BCL_DOUBLE_T(x) BCL_DOUBLE(::bcl::encode(x))

#define BCL_DOUBLE_V(x) BCL_DOUBLE_T(x){}

#if !defined(D_T)
#define D_T(x) BCL_DOUBLE_T(x)
#endif

#if !defined(D_V)
#define D_V(x) BCL_DOUBLE_V(x)
#endif

#if !defined(D_)
#define D_(x) BCL_DOUBLE_V(x)
#endif


// trait and operators

namespace bcl{
	namespace detail{
		template <typename T>
		struct is_encoded_double : ::std::false_type{
		};

		template <bool Negative, ::std::uint64_t Significand, int Exponent>
		struct is_encoded_double<double_<Negative, Significand, Exponent>>
			: ::std::true_type
		{
		};

		template <bool Negative>
		struct is_encoded_double<zero<Negative>> : ::std::true_type{
		};

		template <bool Negative>
		struct is_encoded_double<inf<Negative>> : ::std::true_type{
		};

		template <>
		struct is_encoded_double<qnan> : ::std::true_type{
		};

		template <>
		struct is_encoded_double<snan> : ::std::true_type{
		};

		template <typename T>
		constexpr bool is_encoded_double_v = is_encoded_double<T>::value;

		template <
			typename T, typename U,
			::std::enable_if_t<is_encoded_double_v<T> && is_encoded_double_v<U>>* = nullptr
		>
		constexpr auto operator+(T, U)
		{
			constexpr auto result = encode(T::value + U::value);
			return BCL_DOUBLE(result){};
		}

		template<
			typename T, typename U,
			::std::enable_if_t<is_encoded_double_v<T> && is_encoded_double_v<U>>* = nullptr
		>
		constexpr auto operator-(T, U)
		{
			constexpr auto result = encode(T::value - U::value);
			return BCL_DOUBLE(result){};
		}

		template<
			typename T, typename U,
			::std::enable_if_t<is_encoded_double_v<T> && is_encoded_double_v<U>>* = nullptr
		>
		constexpr auto operator*(T, U)
		{
			constexpr auto result = encode(T::value * U::value);
			return BCL_DOUBLE(result){};
		}

		template<
			typename T, typename U,
			::std::enable_if_t<is_encoded_double_v<T> && is_encoded_double_v<U>>* = nullptr
		>
		constexpr auto operator/(T, U)
		{
			constexpr auto result = encode(T::value / U::value);
			return BCL_DOUBLE(result){};
		}

		template <
			typename T, typename U,
			::std::enable_if_t<is_encoded_double_v<T> && is_encoded_double_v<U>>* = nullptr
		>
		constexpr bool operator==(T, U)
		{
			return T::value == U::value;
		}

		template <
			typename T, typename U,
			::std::enable_if_t<is_encoded_double_v<T> && is_encoded_double_v<U>>* = nullptr
		>
		constexpr bool operator!=(T, U)
		{
			return T::value != U::value;
		}

		template <
			typename T, typename U,
			::std::enable_if_t<is_encoded_double_v<T> && is_encoded_double_v<U>>* = nullptr
		>
		constexpr bool operator<(T, U)
		{
			return T::value < U::value;
		}

		template <
			typename T, typename U,
			::std::enable_if_t<is_encoded_double_v<T> && is_encoded_double_v<U>>* = nullptr
		>
		constexpr bool operator<=(T, U)
		{
			return T::value <= U::value;
		}

		template <
			typename T, typename U,
			::std::enable_if_t<is_encoded_double_v<T> && is_encoded_double_v<U>>* = nullptr
		>
		constexpr bool operator>(T, U)
		{
			return T::value > U::value;
		}

		template <
			typename T, typename U,
			::std::enable_if_t<is_encoded_double_v<T> && is_encoded_double_v<U>>* = nullptr
		>
		constexpr bool operator>=(T, U)
		{
			return T::value >= U::value;
		}
	}

	using detail::is_encoded_double;

	template <typename T>
	constexpr bool is_encoded_double_v = is_encoded_double<T>::value;

	namespace detail{
		template <typename T, ::std::enable_if_t<is_encoded_double<T>{}>* = nullptr>
		constexpr auto encode()
		{
			return encode(T::value);
		}
	}
}

