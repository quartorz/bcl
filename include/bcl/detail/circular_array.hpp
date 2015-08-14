#pragma once

#include <cstdint>

namespace bcl::detail{
	template <typename T, ::std::size_t N>
	class circular_array{
		T a_[N] = {};
		::std::size_t first_ = 0, last_ = 0;
		bool full_ = false;

	public:
		constexpr void push_back(const T &x)
		{
			a_[last_ % N] = x;

			last_ = (last_ + 1) % N;

			if(full_)
				first_ = last_;

			full_ = (first_ == last_);
		}

		constexpr void pop_back()
		{
			if(last_ == 0)
				last_ = N - 1;
			else
				--last_;
		}

		constexpr bool empty() const
		{
			return first_ == last_;
		}

		constexpr ::std::size_t size() const
		{
			if(full_)
				return N;

			if(last_ < first_)
				return N - (first_ - last_) + 1;
			else
				return last_ - first_;
		}

		constexpr bool full() const
		{
			return full_;
		}

		constexpr T &operator[](::std::size_t i)
		{
			return a_[(i + first_) % N];
		}

		constexpr const T &operator[](::std::size_t i) const
		{
			return a_[(i + first_) % N];
		}
	};
}

