#pragma once

#include <cstdint>

#include <bcl/tuple.hpp>
#include <bcl/value_tuple.hpp>

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

	template <typename T, typename Tuple>
	struct tuple_operation_comp_tautology : ::std::true_type{
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
				tuple_operation_comp_tautology,
				detail::tuple_merge_state,
				tuple<Tuples...>,
				tuple<>
			>::state;
	};

	template <typename ... Tuples>
	using tuple_merge_t = typename tuple_merge<Tuples...>::type;
}

