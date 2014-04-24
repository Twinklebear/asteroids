#ifndef BUFFER_OFFSET_H
#define BUFFER_OFFSET_H

#include "buffer_size.h"

namespace detail {
template<int I, Layout L, typename... Args>
struct Offset;
template<int I, typename T, typename... Args>
struct Offset<I, Layout::PACKED, T, Args...> {
	static_assert(I < sizeof...(Args) + 1, "PackedOffset index out of bounds");
	static size_t offset(size_t prev = 0){
		prev += sizeof(T);
		return Offset<I - 1, Layout::PACKED, Args...>::offset(prev);
	}
};
template<typename T, typename... Args>
struct Offset<0, Layout::PACKED, T, Args...> {
	static size_t offset(size_t prev = 0){
		return prev;
	}
};
template<int I, typename T, typename... Args>
struct Offset<I, Layout::ALIGNED, T, Args...> {
	static_assert(I < sizeof...(Args) + 1, "AlignedOffset index out of bounds");
	static size_t offset(size_t prev = 0){
		prev = Size<Layout::ALIGNED, T>::size(prev);
		return Offset<I - 1, Layout::ALIGNED, Args...>::offset(prev);
	}
};
template<typename T, typename... Args>
struct Offset<0, Layout::ALIGNED, T, Args...> {
	static size_t offset(size_t prev = 0){
		prev = prev % alignof(T) == 0 ? prev
			: prev + alignof(T) - prev % alignof(T);
		return prev;
	}
};

}

#endif

