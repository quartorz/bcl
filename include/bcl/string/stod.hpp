#pragma once

#include <iterator>
#include <cstdint>

#include <sprout/ctype.hpp>
#include <sprout/tuple.hpp>
#include <sprout/string.hpp>

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

	// constexpr implementation of kv::conv_double::stringtod
	template <typename String>
	constexpr double stod(String &&s, ::std::size_t *pos = nullptr, int mode = 0)
	{
		using std::begin;
		using std::end;

		auto iter = begin(s);
		auto e = end(s);

		using value_type = typename ::std::iterator_traits<decltype(iter)>::value_type;

		// ignore leading spaces
		while(iter != e && ::sprout::isspace(*iter))
			iter++;

		int sign = 1;
		::sprout::tie(sign, iter) = detail::get_sign(iter, e);

		// integral part
		auto intpart_begin = iter;
		auto intpart_end = iter;

		::sprout::tie(intpart_begin, intpart_end) = detail::get_number(iter, e);
		iter = intpart_end;

		// decimal part
		auto decpart_begin = iter;
		auto decpart_end = iter;

		if(iter != e && *iter == static_cast<value_type>('.')){
			::sprout::tie(decpart_begin, decpart_end) = detail::get_number(iter + 1, e);
			iter = decpart_end;
		}

		// exponent part
		int esign = 1;
		auto exppart_begin = iter;
		auto exppart_end = iter;
		int e10 = 0;

		if(iter != e
			&& (*iter == static_cast<value_type>('e') || *iter == static_cast<value_type>('E')))
		{
			::sprout::tie(esign, iter) = detail::get_sign(iter + 1, e);
			::sprout::tie(exppart_begin, exppart_end) = detail::get_number(iter, e);
			iter = exppart_end;
			e10 = esign * ::sprout::stoi(::sprout::string<10>(exppart_begin, exppart_end));
		}

		while(intpart_begin != intpart_end && *intpart_begin == static_cast<value_type>('0'))
			++intpart_begin;
		
		while(decpart_end != decpart_begin && *decpart_end == static_cast<value_type>('0'))
			--decpart_end;

		int table[100] = {};
		int table_max = (intpart_end - intpart_begin) - 1 + e10;
		int table_min = -static_cast<int>(decpart_end - decpart_begin) + e10;
		int offset = -table_min;
	}
}

