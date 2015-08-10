#pragma once

#include <utility>
#include <cstdint>
#include <type_traits>

#include <sprout/index_tuple.hpp>

#include <bcl/has_xxx.hpp>

namespace bcl{
	template <typename T, T ...>
	struct value_tuple;
}

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
			constexpr tuple_base(const tuple_base &) = default;
			constexpr tuple_base(tuple_base&&) = default;

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

	namespace detail{
		template <typename T>
		struct is_tuple : ::std::false_type{
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
	private:
		using base_type = detail::tuple_base<::sprout::index_range<0, sizeof...(Ts)>, Ts...>;

	public:
		tuple() = default;
		tuple(const tuple&) = default;
		tuple(tuple &&) = default;

		template <
			typename Arg, typename ... Args,
			::std::enable_if_t<
				sizeof...(Ts) == (sizeof...(Args) + 1)
				&& (sizeof...(Args) == 0 && !detail::is_tuple<Arg>{})
			>* = nullptr
		>
		tuple(Arg &&arg, Args &&... args)
			: base_type(::std::forward<Arg>(arg), ::std::forward<Args>(args)...)
		{
		}

		//using detail::tuple_base<::sprout::index_range<0, sizeof...(Ts)>, Ts...>::tuple_base;

		template <typename ... Us>
		tuple(const tuple<Us...> &t)
			: tuple(static_cast<tuple<Ts...>>(t))
		{
		}

	private:
		template <typename ... Us, ::sprout::index_t ... Is>
		constexpr auto cast_impl(::sprout::index_tuple<Is...>) const
		{
			return tuple<Us...>(static_cast<Us>(this->template get<Is>())...);
		}

	public:
		template <typename ... Us>
		constexpr operator tuple<Us...>() const
		{
			return cast_impl<Us...>(::sprout::make_index_tuple<sizeof...(Us)>::make());
		}
	};

	template <>
	struct tuple<>{
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
	namespace detail{
		template <typename Tuple>
		struct tuple_size_impl;

		template <typename ... Types>
		struct tuple_size_impl<tuple<Types...>>{
			constexpr operator ::std::size_t() { return value; }

			static constexpr ::std::size_t value = sizeof...(Types);

			using type = tuple_size_impl;
		};
	}

	/*! @class tuple_size
	    @brief get the number of elements of Tuple
	 */
	template <typename Tuple>
	using tuple_size = typename detail::tuple_size_impl<::std::decay_t<Tuple>>::type;
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

namespace bcl{
	template <typename T>
	struct tuple_from;

	template <template <typename ...> typename T, typename ... Ts>
	struct tuple_from<T<Ts...>>{
		using type = tuple<Ts...>;
	};

	template <typename T>
	using tuple_from_t = typename tuple_from<T>::type;
}

namespace bcl{
	namespace detail{
		template <
			template <typename T, typename Tuple> typename Comp,
			template <bool Cond, typename T, typename Tuple> typename State,
			typename TupleA, typename TupleB,
			::std::size_t Begin, ::std::size_t N,
			typename = void
		>
		class tuple_operation_impl{
			using left = tuple_operation_impl<Comp, State, TupleA, TupleB, Begin, N / 2 + N % 2>;
			using right = tuple_operation_impl<
				Comp, State, TupleA, typename left::state,
				Begin + N / 2 + N % 2, N / 2
			>;

		public:
			using type = ::bcl::tuple_concat_t<typename left::type, typename right::type>;
			using state = typename right::state;
		};

		template <
			template <typename T, typename Tuple> typename Comp,
			template <bool Cond, typename T, typename Tuple> typename State,
			typename TupleA, typename TupleB,
			::std::size_t Begin
		>
		struct tuple_operation_impl<Comp, State, TupleA, TupleB, Begin, 0, void>{
			using type = tuple<>;
			using state = TupleB;
		};

		template <
			template <typename T, typename Tuple> typename Comp,
			template <bool Cond, typename T, typename Tuple> typename State,
			typename TupleA, typename TupleB,
			::std::size_t Begin
		>
		struct tuple_operation_impl<
			Comp, State, TupleA, TupleB, Begin, 1,
			::std::enable_if_t<!Comp<::bcl::tuple_element_t<Begin, TupleA>, TupleB>::value>
		>{
			using type = tuple<>;
			using state = typename State<false, ::bcl::tuple_element_t<Begin, TupleA>, TupleB>::type;
		};

		template <
			template <typename T, typename Tuple> typename Comp,
			template <bool Cond, typename T, typename Tuple> typename State,
			typename TupleA, typename TupleB,
			::std::size_t Begin
		>
		struct tuple_operation_impl<
			Comp, State, TupleA, TupleB, Begin, 1,
			::std::enable_if_t<Comp<::bcl::tuple_element_t<Begin, TupleA>, TupleB>::value>
		>{
			using type = tuple<::bcl::tuple_element_t<Begin, TupleA>>;
			using state = typename State<true, ::bcl::tuple_element_t<Begin, TupleA>, TupleB>::type;
		};
	}

	template <
		template <typename T, typename Tuple> typename Comp,
		template <bool Cond, typename T, typename Tuple> typename State,
		typename TupleA, typename TupleB
	>
	struct tuple_operation
		: detail::tuple_operation_impl<Comp, State, TupleA, TupleB, 0, tuple_size<TupleA>::value>
	{
	};
}

namespace bcl{
	namespace detail{
		template <typename T, typename Tuple>
		struct tuple_common_element_comp{
			static constexpr bool value = ::bcl::has_value_v<::bcl::tuple_find_t<T, Tuple>>;
		};

		template <bool Cond, typename T, typename Tuple>
		class tuple_common_element_state{
			// Cond is true

			static constexpr auto index = ::bcl::tuple_find_t<T, Tuple>::value;

		public:
			using type = ::bcl::tuple_remove_t<::bcl::value_tuple<::std::size_t, index>, Tuple>;
		};

		template <typename T, typename Tuple>
		struct tuple_common_element_state<false, T, Tuple>{
			using type = Tuple;
		};
	}

	template <typename TupleA, typename TupleB>
	using tuple_common_element = tuple_operation<
		detail::tuple_common_element_comp,
		detail::tuple_common_element_state,
		TupleA, TupleB
	>;

	template <typename TupleA, typename TupleB>
	using tuple_common_element_t = typename tuple_common_element<TupleA, TupleB>::type;
}

namespace bcl{
	namespace detail{
		template <typename TupleA, typename Common>
		struct tuple_element_subtract_impl;

		template <typename TupleA, typename ... Common>
		struct tuple_element_subtract_impl<TupleA, tuple<Common...>>{
			using type = tuple_remove_t<
				::bcl::value_tuple<
					::std::size_t,
					::bcl::tuple_find_t<Common, TupleA>::value...>,
				TupleA
			>;
		};
	}

	template <typename TupleA, typename TupleB>
	struct tuple_element_subtract
		: detail::tuple_element_subtract_impl<TupleA, tuple_common_element_t<TupleA, TupleB>>
	{
	};

	template <typename TupleA, typename TupleB>
	using tuple_element_subtract_t = typename tuple_element_subtract<TupleA, TupleB>::type;
}

namespace bcl{
	namespace detail{
		template <typename T, typename Tuple>
		struct tuple_merge_comp : ::std::true_type{
		};

		template <bool, typename T, typename Tuple>
		struct tuple_merge_state{
			using type = ::bcl::tuple_concat_t<T, Tuple>;
		};
	}

	template <typename ... Tuples>
	struct tuple_merge{
		using type =
			typename tuple_operation<
				detail::tuple_merge_comp,
				detail::tuple_merge_state,
				tuple<Tuples...>,
				tuple<>
			>::state;
	};

	template <typename ... Tuples>
	using tuple_merge_t = typename tuple_merge<Tuples...>::type;
}

namespace bcl{
	namespace detail{
		template <
			template <typename T> typename Func, typename Tuple,
			typename Result, ::std::size_t I, ::std::size_t N
		>
		class tuple_map_impl{
			using left = tuple_map_impl<Func, Tuple, Result, I, N / 2 + N % 2>;
			using right = tuple_map_impl<
				Func, Tuple, typename left::type, I + N / 2 + N % 2, N / 2
			>;

		public:
			using type = typename right::type;
		};

		template <
			template <typename T> typename Func, typename Tuple,
			typename Result, ::std::size_t I
		>
		class tuple_map_impl<Func, Tuple, Result, I, 1>{
			using result = typename Func<::bcl::tuple_element_t<I, Tuple>>::type;

		public:
			using type = ::bcl::tuple_concat_t<Result, tuple<result>>;
		};
	}

	template <template <typename T> typename Func, typename Tuple>
	struct tuple_map : detail::tuple_map_impl<
		Func, Tuple, tuple<>, 0, tuple_size<Tuple>::value
	>
	{
	};

	template <template <typename T> typename Func, typename Tuple>
	using tuple_map_t = typename tuple_map<Func, Tuple>::type;
}

namespace bcl{
	namespace detail{
		template <
			typename TupleA, typename TupleB,
			typename Result, ::std::size_t I, ::std::size_t N
		>
		class tuple_cartesian_prod_impl{
			using left = tuple_cartesian_prod_impl<
				TupleA, TupleB, Result, I, N / 2 + N % 2
			>;
			using right = tuple_cartesian_prod_impl<
				TupleA, TupleB, typename left::type, I + N / 2 + N % 2, N / 2
			>;

		public:
			using type = typename right::type;
		};

		template <
			typename TupleA, typename TupleB,
			typename Result, ::std::size_t I
		>
		class tuple_cartesian_prod_impl<TupleA, TupleB, Result, I, 1>{
			using element = ::bcl::tuple_element_t<I, TupleA>;

			template <typename T>
			struct func{
				using type = ::bcl::tuple_concat_t<element, T>;
			};

		public:
			using type = ::bcl::tuple_concat_t<Result, ::bcl::tuple_map_t<func, TupleB>>;
		};
	}

	template <typename TupleA, typename TupleB>
	struct tuple_cartesian_prod : detail::tuple_cartesian_prod_impl<
		TupleA, TupleB, tuple<>, 0, tuple_size<TupleA>::value
	>{
	};

	template <typename TupleA, typename TupleB>
	using tuple_cartesian_prod_t = typename tuple_cartesian_prod<TupleA, TupleB>::type;
}

namespace bcl{
	namespace detail{
		template <typename Tuple, typename Result, ::std::size_t I, ::std::size_t N>
		class tuple_cartesian_prod_variadic_impl{
			using left = tuple_cartesian_prod_variadic_impl<
				Tuple, Result, I, N / 2 + N % 2
			>;
			using right = tuple_cartesian_prod_variadic_impl<
				Tuple, typename left::type, I + N / 2 + N % 2, N / 2
			>;

		public:
			using type = typename right::type;
		};

		template <typename Tuple, typename Result, ::std::size_t I>
		struct tuple_cartesian_prod_variadic_impl<Tuple, Result, I, 1>{
			using type = ::bcl::tuple_cartesian_prod_t<
				Result, ::bcl::tuple_element_t<I, Tuple>
			>;
		};
	}

	template <typename ... Tuples>
	struct tuple_cartesian_prod_variadic{
		using type = tuple<>;
	};

	template <typename Tuple>
	struct tuple_cartesian_prod_variadic<Tuple>{
		using type = Tuple;
	};

	template <typename Head, typename ... Rest>
	struct tuple_cartesian_prod_variadic<Head, Rest...> : detail::tuple_cartesian_prod_variadic_impl<
		tuple<Rest...>, Head, 0, sizeof...(Rest)
	>{
	};

	template <typename ... Tuples>
	using tuple_cartesian_prod_variadic_t = typename tuple_cartesian_prod_variadic<Tuples...>::type;
}

namespace bcl{
	namespace detail{
		template <::std::size_t I, typename T, typename Tuple, ::std::size_t C, ::std::size_t N>
		class tuple_replace_impl{
			using left = tuple_replace_impl<
				I, T, Tuple, C, N / 2 + N % 2
			>;
			using right = tuple_replace_impl<
				I, T, Tuple, C + N / 2 + N % 2, N / 2
			>;


		public:
			using type = ::bcl::tuple_concat_t<typename left::type, typename right::type>;
		};

		template <::std::size_t I, typename T, typename Tuple, ::std::size_t C>
		struct tuple_replace_impl<I, T, Tuple, C, 0>{
			using type = ::bcl::tuple<>;
		};

		template <::std::size_t I, typename T, typename Tuple>
		struct tuple_replace_impl<I, T, Tuple, I, 1>{
			using type = ::bcl::tuple<T>;
		};

		template <::std::size_t I, typename T, typename Tuple, ::std::size_t C>
		struct tuple_replace_impl<I, T, Tuple, C, 1>{
			using type = ::bcl::tuple<::bcl::tuple_element_t<C, Tuple>>;
		};
	}

	template <::std::size_t I, typename T, typename Tuple>
	struct tuple_replace : detail::tuple_replace_impl<
		I, T, Tuple, 0, ::bcl::tuple_size<Tuple>::value
	>{
	};

	template <::std::size_t I, typename T, typename Tuple>
	using tuple_replace_t = typename tuple_replace<I, T, Tuple>::type;
}

namespace bcl{
	namespace detail{
		struct tuple_intersection_null{};

		template <
			typename TupleA, typename TupleB,
			::std::size_t I, ::std::size_t N,
			typename = void
		>
		class tuple_intersection_impl{
			using left = tuple_intersection_impl<
				TupleA, TupleB, I, N / 2 + N % 2
			>;
			using right = tuple_intersection_impl<
				TupleA, typename left::state, I + N / 2 + N % 2, N / 2
			>;

		public:
			using state = typename right::state;
			using type = ::bcl::tuple_concat_t<
				typename left::type, typename right::type
			>;
		};

		template <
			typename TupleA, typename TupleB,
			::std::size_t I
		>
		struct tuple_intersection_impl<TupleA, TupleB, I, 0, void>{
			using state = TupleB;
			using type = ::bcl::tuple<>;
		};

		template <
			typename TupleA, typename TupleB,
			::std::size_t I
		>
		class tuple_intersection_impl<
			TupleA, TupleB, I, 1, ::std::enable_if_t<
				::bcl::has_value_v<::bcl::tuple_find_t<
					::bcl::tuple_element_t<I, TupleA>,
					TupleB
				>>
			>
		>{
			static constexpr auto index = ::bcl::tuple_find_t<
				::bcl::tuple_element_t<I, TupleA>, TupleB
			>::value;

		public:
			using state = ::bcl::tuple_replace_t<
				index, tuple_intersection_null, TupleB
			>;
			using type = ::bcl::tuple<::bcl::tuple_element_t<I, TupleA>>;
		};

		template <
			typename TupleA, typename TupleB,
			::std::size_t I
		>
		class tuple_intersection_impl<
			TupleA, TupleB, I, 1, ::std::enable_if_t<
				!::bcl::has_value_v<::bcl::tuple_find_t<
					::bcl::tuple_element_t<I, TupleA>,
					TupleB
				>>
			>
		>{
			using state = TupleB;
			using type = ::bcl::tuple<>;
		};

	}

	template <typename TupleA, typename TupleB>
	struct tuple_intersection{
	};
}

