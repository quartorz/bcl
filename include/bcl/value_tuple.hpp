#pragma once

#include <utility>
#include <type_traits>

#include <bcl/tuple.hpp>
#include <bcl/has_xxx.hpp>

// value_tuple

namespace bcl{
	namespace detail{
		template <::std::size_t I, typename T, T N>
		struct value_holder{
			static constexpr T value = N;
		};

		template <typename Seq, typename T, T ... Holders>
		struct value_tuple_base;

		template <::std::size_t ... Is, typename T, T ... Ns>
		struct value_tuple_base<
			::std::index_sequence<Is...>,
			T,
			Ns...
		> : value_holder<Is, T, Ns>...
		{
			using value_type = T;

			template <::std::size_t I>
			static constexpr T value = decltype(
					::std::declval<value_tuple_base>().template value_impl<I>())::value;

			template <::std::size_t I>
			constexpr auto get() const
			{
				return decltype(get_impl<I>(*this))::value;
			}

		private:
			template <::std::size_t I>
			constexpr auto value_impl() const
			{
				return get_impl<I>(*this);
			}

			template <::std::size_t I, T N>
			constexpr value_holder<I, T, N> get_impl(const value_holder<I, T, N> & x) const
			{
				return {};
			}
		};
	}

	template <typename T, T ... Ns>
	struct value_tuple
		: detail::value_tuple_base<
		  ::std::make_index_sequence<sizeof...(Ns)>, T, Ns...>
	{
		template <typename U>
		using cast = value_tuple<U, static_cast<U>(Ns)...>;
	};

	template <::std::size_t I, typename T, T ... Ns>
	constexpr auto get(const value_tuple<T, Ns...> &x)
	{
		return x.template get<I>();
	}

	template <::std::size_t ... Is>
	using index_tuple = value_tuple<::std::size_t, Is...>;
}

// tuple_size and tuple_element

namespace bcl{
	namespace detail{
		template <typename T, T ... Ns>
		struct tuple_size_impl<value_tuple<T, Ns...>>{
			constexpr operator ::std::size_t() { return value; }

			static constexpr ::std::size_t value = sizeof...(Ns);

			using type = tuple_size_impl;
		};
	}

	template <typename Tuple>
	using value_tuple_size = tuple_size<Tuple>;

	template <::std::size_t I, typename T, T ... Ns>
	struct tuple_element<I, value_tuple<T, Ns...>>{
		static constexpr T value = value_tuple<T, Ns...>::template value<I>;
	};

	template <::std::size_t I, typename VTuple>
	using value_tuple_element = tuple_element<I, VTuple>;
}

// value_tuple_find

namespace bcl{
	namespace detail{
		template <typename T, T V, typename VTuple, ::std::size_t Begin, ::std::size_t N, typename = void>
		struct value_tuple_find_impl{
			using type = typename ::std::conditional_t<
				::bcl::has_value_v<value_tuple_find_impl<T, V, VTuple, Begin, N / 2 + N % 2>>,
				value_tuple_find_impl<T, V, VTuple, Begin, N / 2 + N % 2>,
				value_tuple_find_impl<T, V, VTuple, Begin + N / 2 + N % 2, N / 2>>::type;
		};

		template <typename T, T V, typename VTuple, ::std::size_t Begin>
		struct value_tuple_find_impl<T, V, VTuple, Begin, 0, void>{
			using type = value_tuple_find_impl;
		};

		template <typename T, T V, typename VTuple, ::std::size_t Begin>
		struct value_tuple_find_impl<
			T, V, VTuple, Begin, 1,
			::std::enable_if_t<V == VTuple::template value<Begin>>>
		{
			using type = value_tuple_find_impl;
			static constexpr ::std::size_t value = Begin;
		};
		
		template <typename T, T V, typename VTuple, ::std::size_t Begin>
		struct value_tuple_find_impl<
			T, V, VTuple, Begin, 1,
			::std::enable_if_t<V != VTuple::template value<Begin>>>
		{
			using type = value_tuple_find_impl;
		};
 	}

	template <typename T, T V, typename VTuple>
	using value_tuple_find = detail::value_tuple_find_impl<T, V, VTuple, 0, tuple_size<VTuple>::value>;

	template <typename T, T V, typename VTuple>
	using value_tuple_find_t = typename value_tuple_find<T, V, VTuple>::type;
}

// tuple_concat

namespace bcl{
	template <typename T, T ... N, T ... M>
	struct tuple_concat<value_tuple<T, N...>, value_tuple<T, M...>>{
		using type = value_tuple<T, N..., M...>;
	};

	template <typename VTupleA, typename VTupleB>
	using value_tuple_concat = tuple_concat<VTupleA, VTupleB>;

	template <typename VTupleA, typename VTupleB>
	using value_tuple_concat_t = typename value_tuple_concat<VTupleA, VTupleB>::type;
}

