#pragma once

#include <iterator>
#include <limits>
#include <cstdint>

#include <sprout/ctype.hpp>
#include <sprout/tuple.hpp>
#include <sprout/string.hpp>
#include <sprout/detail/char_literal.hpp>

#include <bcl/math/pow2.hpp>
#include <bcl/detail/circular_array.hpp>

namespace bcl{
	namespace detail{
		template <typename Iter>
		constexpr auto get_sign(Iter iter, Iter end)
		{
			using value_type = typename ::std::iterator_traits<Iter>::value_type;

			int sign = 1;

			if(iter != end){
				if(*iter == static_cast<value_type>('-')){
					sign = -1;
					++iter;
				}else if(*iter == static_cast<value_type>('+')){
					++iter;
				}
			}

			return ::sprout::make_tuple(sign, ::std::move(iter));
		}

		template <typename Iter>
		constexpr auto get_number(Iter iter, Iter end)
		{
			using value_type = typename ::std::iterator_traits<Iter>::value_type;

			Iter begin = iter;

			while(iter != end && ::sprout::isdigit(*iter))
				++iter;

			return ::sprout::make_tuple(::std::move(begin), ::std::move(iter));
		}
	}

	template <typename String>
	constexpr double stod(String &&s, ::std::size_t *pos = nullptr, int mode = 0)
	{
		using ::sprout::begin;
		using ::sprout::end;

		auto iter = begin(s);
		auto e = end(s);

		using value_type = typename ::std::iterator_traits<decltype(iter)>::value_type;

		// ignore leading spaces
		while(iter != e && ::sprout::isspace(*iter))
			iter++;

		int sign = 1;
		::sprout::tie(sign, iter) = detail::get_sign(iter, e);

		// integer part of input
		auto intpart_begin = iter;
		auto intpart_end = iter;

		::sprout::tie(intpart_begin, intpart_end) = detail::get_number(iter, e);
		iter = intpart_end;

		// fraction part of input
		auto fracpart_begin = iter;
		auto fracpart_end = iter;

		if(iter != e && *iter == SPROUT_CHAR_LITERAL('.', value_type)){
			::sprout::tie(fracpart_begin, fracpart_end) = detail::get_number(iter + 1, e);
			iter = fracpart_end;
		}

		// exponent part of input
		int esign = 1;
		auto exppart_begin = iter;
		auto exppart_end = iter;
		int e10 = 0;

		if(iter != e
			&& (*iter == SPROUT_CHAR_LITERAL('e', value_type)
				|| *iter == SPROUT_CHAR_LITERAL('E', value_type)))
		{
			::sprout::tie(esign, iter) = detail::get_sign(iter + 1, e);
			::sprout::tie(exppart_begin, exppart_end) = detail::get_number(iter, e);
			iter = exppart_end;
			e10 = esign * ::sprout::stoi(::sprout::string<10>(exppart_begin, exppart_end));
		}

		if(pos != nullptr)
			*pos = (iter - begin(s));

		// ignore leading zeros of integer part
		while(intpart_begin != intpart_end && *intpart_begin == SPROUT_CHAR_LITERAL('0', value_type))
			++intpart_begin;

		// ignore trailing zeros of fraction part
		while(fracpart_end != fracpart_begin && *(fracpart_end - 1) == SPROUT_CHAR_LITERAL('0', value_type))
			--fracpart_end;

		// overflow
		if((intpart_end - intpart_begin) + e10 - 1 > 308)
			return sign * ::std::numeric_limits<double>::infinity();

		// count of ones in input number converted to binary
		int one_count = 0;

		// convert integer part from decimal to binary
		int int_dec_digits = intpart_end - intpart_begin;
		int int_dec[309] = {};
		int int_bin_digits = 0;
		detail::circular_array<int, 54> int_bin{};

		for(int i = int_dec_digits - 1; i >= 0 && (i + e10) >= 0; --i){
			int_dec[i + e10] = *intpart_begin++ - SPROUT_CHAR_LITERAL('0', value_type);
		}

		for(int i = e10 - 1; i >= 0 && fracpart_begin != fracpart_end; --i){
			int_dec[i] = *fracpart_begin++ - SPROUT_CHAR_LITERAL('0', value_type);
		}

		{
			int int_dec_end = int_dec_digits + e10;

			while(int_dec_end > 0){
				int remainder = 0;

				for(int i = int_dec_end - 1; i > 0; --i){
					int n = (remainder * 10 + int_dec[i]) / 2;
					remainder = (remainder * 10 + int_dec[i]) % 2;
					int_dec[i] = n;
				}

				int n = (remainder * 10 + int_dec[0]) / 2;
				remainder = (remainder * 10 + int_dec[0]) % 2;
				int_dec[0] = n;

				int_bin.push_back(remainder);
				int_bin_digits++;

				if(remainder == 1)
					one_count++;

				if(int_dec[int_dec_end - 1] == 0)
					int_dec_end--;
			}
		}

		if(int_bin_digits > 1024)
			return sign * ::std::numeric_limits<double>::infinity();

		// convert fraction part from decimal to binary
		int frac_dec_digits = (-e10 <= 0) ? 0 : -e10 - 1;
		int frac_dec[325 + 16] = {};
		detail::circular_array<int, 54> frac_bin{};
		int frac_bin_exp = 0;

		while(intpart_begin != intpart_end && frac_dec_digits < 324 + 16){
			frac_dec[frac_dec_digits++] = *intpart_begin++ - SPROUT_CHAR_LITERAL('0', value_type);
		}

		while(fracpart_begin != fracpart_end && frac_dec_digits < 324 + 16){
			frac_dec[frac_dec_digits++] = *fracpart_begin++ - SPROUT_CHAR_LITERAL('0', value_type);
		}

		while(frac_dec_digits != 0){
			int carry = 0;

			for(int i = frac_dec_digits - 1; i > 0; --i){
				int n = frac_dec[i] * 2 + carry;
				frac_dec[i] = n % 10;
				carry = n / 10;
			}

			int n = frac_dec[0] * 2 + carry;
			frac_dec[0] = n % 10;
			carry = n / 10;

			if(frac_bin.size() == 0 && carry == 0)
				frac_bin_exp--;
			else{
				frac_bin.push_back(carry);
				if(carry == 1)
					one_count++;
			}

			if(frac_bin.size() == 54)
				break;

			if(frac_dec[frac_dec_digits - 1] == 0)
				frac_dec_digits--;
		}

		// check whether residue of decimal fraction part is zero or not
		bool frac_dec_zero = (frac_dec_digits == 0);

		// determine round mode
		bool round_up = false;

		int frac_lowest_idx = 53 - int_bin.size();

		if(int_bin.size() == 54){
			if(int_bin[0] == 1){
				if(mode >= 0)
					round_up = true;
			}else{
				if(mode > 0){
					if(!frac_dec_zero)
						round_up = true;
					else{
						int one = 0;

						for(int i = 53; i > 0; --i){
							if(int_bin[i] == 1)
								one++;
						}

						if(one != one_count)
							round_up = true;
					}
				}
			}
		}else{
			if(frac_lowest_idx >= 0){
				if(frac_bin[frac_lowest_idx] == 1){
					if(mode >= 0)
						round_up = true;
				}
			}

			if(mode > 0){
				if(!frac_dec_zero)
					round_up = true;
				else{
					int one = 0;

					for(int i = 0; i < int_bin.size(); ++i){
						if(int_bin[i] == 1)
							one++;
					}

					for(int i = 0; i < frac_lowest_idx; ++i){
						if(frac_bin[i] == 1)
							one++;
					}

					if(one != one_count)
						round_up = true;
				}
			}
		}

		if(sign == -1 && mode != 0){
			round_up = !round_up;
		}

		// make integer part
		double intpart = 0.0;

		if(int_bin.size() >= 53){
			int one = 0;

			for(int i = 53, j = (int_bin.size() == 54 ? 1 : 0); i >= 0; --i){
				intpart *= 2.0;
				intpart += static_cast<double>(int_bin[i + j]);

				if(int_bin[i + j] == 1)
					one++;
			}

			if(one == 53 && int_bin_digits == 1024 && round_up)
				return sign * ::std::numeric_limits<double>::infinity();

			if(round_up)
				intpart += 1.0;

			return sign * intpart * pow2(int_bin_digits - 53);
		}

		for(int i = int_bin.size() - 1; i >= 0; --i){
			intpart *= 2.0;
			intpart += static_cast<double>(int_bin[i]);
		}

		intpart *= pow2(int_bin_digits - int_bin.size());

		// make fraction part
		double fracpart = 0.0;

		if(frac_bin_exp < -1022)
			frac_lowest_idx -= -1022 - frac_bin_exp;

		for(int i = 0; i < frac_lowest_idx - 1; ++i){
			fracpart *= 2.0;
			fracpart += static_cast<double>(frac_bin[i]);
		}

		if(round_up)
			fracpart += 1.0;

		fracpart /= pow2(frac_lowest_idx - 1);

		for(int i = -frac_bin_exp; i > 0; --i)
			fracpart /= 2.0;

		// combine integer part and fraction part
		return sign * (intpart + fracpart);
	}
}

