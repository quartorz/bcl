#pragma once

namespace bcl{
	template <typename T, template <typename ...> typename U>
	struct replace;

	template <typename ... Ts, template <typename ...> typename T, template <typename ...> typename U>
	struct replace<T<Ts...>, U>{
		using type = U<Ts...>;
	};

	template <typename T, template <typename ...> typename U>
	using replace_t = typename replace<T, U>::type;
}

