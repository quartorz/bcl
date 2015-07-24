#pragma once

#include <utility>
#include <cstdint>
#include <type_traits>

#include <sprout/index_tuple.hpp>

#include <bcl/has_xxx.hpp>

// definitions of bcl::tuple and bcl::get<I>

namespace bcl{
	namespace detail{
		template <::sprout::index_t I, typename T>
		class holder{
			T value;

		public:
			constexpr holder() = default;

			constexpr holder(T a)
				: value(a)
			{
			}

			constexpr T operator()() const
			{
				return value;
			}

			T &operator()()
			{
				return value;
			}
		};

		template <typename Seq, typename ... Types>
		struct tuple_base;

		template <::sprout::index_t ... Is, typename... Types>
		struct tuple_base<::sprout::index_tuple<Is...>, Types...>
			: holder<Is, Types>...
		{
			constexpr tuple_base() = default;

			template <typename ... Args, typename = ::std::enable_if_t<sizeof...(Args) != 0>>
			constexpr tuple_base(Args && ... args)
				: holder<Is, Types>(::std::forward<Args>(args))...
			{
			}

			template <::sprout::index_t I>
			constexpr auto get() const
			{
				return get_impl<I>(*this);
			}

			template <::sprout::index_t I>
			auto &get()
			{
				return get_impl<I>(*this);
			}

		private:
			template <::sprout::index_t I, typename T>
			constexpr auto get_impl(const holder<I, T> &x) const
			{
				return x();
			}

			template <::sprout::index_t I, typename T>
			auto &get_impl(holder<I, T> &x)
			{
				return x();
			}
		};
	}

	/*! @class bcl::tuple<Types...>
	    @brief simple constexpr tuple
		       instantiation depth increases O(log2(sizeof...Types))
	 */
	template <typename ... Ts>
	struct tuple
		: detail::tuple_base<::sprout::index_range<0, sizeof...(Ts)>, Ts...>
	{
		using detail::tuple_base<::sprout::index_range<0, sizeof...(Ts)>, Ts...>::tuple_base;
	};

	/*! @brief equivalent to std::get<I>
	 */
	template <::sprout::index_t I, typename ... Ts>
	constexpr auto get(const tuple<Ts...> &x)
	{
		return x.template get<I>();
	}

	template <::sprout::index_t I, typename ... Ts>
	auto &get(tuple<Ts...> &x)
	{
		return x.template get<I>();
	}
}

// definition of tuple_element<I, Tuple>

namespace bcl{
	/*! @class tuple_element
	    @brief get I-th element of Tuple
	 */
	template <::std::size_t I, typename Tuple>
	struct tuple_element{
		using type = decltype(get<I>(::std::declval<Tuple>()));
	};

	template <::std::size_t I, typename Tuple>
	using tuple_element_t = typename tuple_element<I, Tuple>::type;
}

// tuple_size<Tuple>

namespace bcl{
	/*! @class tuple_size
	    @brief get the number of elements of Tuple
	 */
	template <typename Tuple>
	struct tuple_size;

	template <typename ... Types>
	struct tuple_size<tuple<Types...>>{
		constexpr operator ::std::size_t() { return value; }

		static constexpr ::std::size_t value = sizeof...(Types);
	};
}

// definition of tuple_find<Tuple, T>

namespace bcl{
	namespace detail{
		template <typename Tuple, typename T, ::std::size_t Begin, ::std::size_t N, typename = void>
		struct tuple_find_impl
		{
			using type = typename ::std::conditional_t<
				::bcl::has_value_v<typename tuple_find_impl<Tuple, T, Begin, N / 2 + N % 2>::type>,
				tuple_find_impl<Tuple, T, Begin, N / 2 + N % 2>,
				tuple_find_impl<Tuple, T, Begin + N / 2 + N % 2, N / 2>
			>::type;
		};

		template <typename Tuple, typename T, ::std::size_t Begin>
		struct tuple_find_impl<Tuple, T, Begin, 0, void>{
			using type = tuple_find_impl;
		};

		template <typename Tuple, typename T, ::std::size_t Begin>
		struct tuple_find_impl<
			Tuple,
			T,
			Begin,
			1,
			::std::enable_if_t<std::is_same<T, ::bcl::tuple_element_t<Begin, Tuple>>{}>
		>
		{
			using type = tuple_find_impl;
			static constexpr ::std::size_t value = Begin;
		};

		template <typename Tuple, typename T, ::std::size_t Begin>
		struct tuple_find_impl<
			Tuple,
			T,
			Begin,
			1,
			::std::enable_if_t<!std::is_same<T, ::bcl::tuple_element_t<Begin, Tuple>>{}>
		>
		{
			using type = tuple_find_impl;
		};
	}

	/*! @brief find T from Tuple 
	           if T was found, tuple_find::value will be defined and have the index of T in Tuple
			   otherwise, tuple_find::value will not be defined
	 */
	template <typename T, typename Tuple>
	using tuple_find = detail::tuple_find_impl<Tuple, T, 0, tuple_size<Tuple>::value>;

	template <typename T, typename Tuple>
	using tuple_find_t = typename tuple_find<T, Tuple>::type;
}

// tuple_concat

namespace bcl{
	template <typename TupleA, typename TupleB>
	struct tuple_concat;

	template <typename ... T, typename ... U>
	struct tuple_concat<tuple<T...>, tuple<U...>>{
		using type = tuple<T..., U...>;
	};

	template <typename TupleA, typename TupleB>
	using tuple_concat_t = typename tuple_concat<TupleA, TupleB>::type;
}

// tuple_remove

namespace bcl{
	namespace detail{
		// this is forward declaration
		// definition is in bcl/value_tuple.hpp
		template <typename T, T V, typename VTuple, ::std::size_t, ::std::size_t, typename>
		struct value_tuple_find_impl;

		template <
			typename VTuple, ::std::size_t SizeOfVTuple,
			typename Tuple, ::std::size_t Begin, ::std::size_t N, typename = void
		>
		struct tuple_remove_impl{
			using type = tuple_concat_t<
				typename tuple_remove_impl<VTuple, SizeOfVTuple, Tuple, Begin, N / 2 + N % 2>::type,
				typename tuple_remove_impl<VTuple, SizeOfVTuple, Tuple, Begin + N / 2 + N % 2, N / 2>::type
			>;
		};

		template <
			typename VTuple, ::std::size_t Size,
			typename Tuple, ::std::size_t Begin
		>
		struct tuple_remove_impl<VTuple, Size, Tuple, Begin, 0, void>
		{
			using type = tuple<>;
		};

		template <
			typename VTuple, ::std::size_t Size,
			typename Tuple, ::std::size_t Begin
		>
		struct tuple_remove_impl<
			VTuple, Size, Tuple, Begin, 1,
			::std::enable_if_t<
				!::bcl::has_value_v<
					typename value_tuple_find_impl<::std::size_t, Begin, VTuple, 0, Size, void>::type
				>
			>
		>
		{
			using type = tuple<::bcl::tuple_element_t<Begin, Tuple>>;
		};

		template <
			typename VTuple, ::std::size_t Size,
			typename Tuple, ::std::size_t Begin
		>
		struct tuple_remove_impl<
			VTuple, Size, Tuple, Begin, 1,
			::std::enable_if_t<
				::bcl::has_value_v<
					typename value_tuple_find_impl<::std::size_t, Begin, VTuple, 0, Size, void>::type
				>
			>
		>
		{
			using type = tuple<>;
		};
	}

	/*! @class tuple_remove
	    @brief remove tuple<...> elements whose index is in value_tuple<T, ...>
	           (value_tuple is defined in bcl/value_tuple.hpp)
	 */
	template <typename VTuple, typename Tuple>
	struct tuple_remove
		: detail::tuple_remove_impl<
			typename VTuple::template cast<::std::size_t>, tuple_size<VTuple>::value,
			Tuple, 0, tuple_size<Tuple>::value
		>
	{
	};

	template <typename VTuple, typename Tuple>
	using tuple_remove_t = typename tuple_remove<VTuple, Tuple>::type;
}

namespace bcl{
	template <template <typename ...> class T, typename Tuple>
	struct tuple_transform;

	template <template <typename ...> class T, typename ... Types>
	struct tuple_transform<T, tuple<Types...>>{
		using type = T<Types...>;
	};

	template <template <typename ...> class T, typename Tuple>
	using tuple_transform_t = typename tuple_transform<T, Tuple>::type;
}

