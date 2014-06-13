#ifndef BUFFER_OFFSET_H
#define BUFFER_OFFSET_H

#include "std140_array.h"
#include "layout_size.h"

namespace detail {
template<Layout L, typename T, typename... Args>
struct Offset {
	static std::array<size_t, 1 + sizeof...(Args)> offsets(){
		std::array<size_t, sizeof...(Args) + 1> arr;
		fill_offsets(arr, 0);
		return arr;
	}
	template<size_t N>
	static void fill_offsets(std::array<size_t, N> &arr, size_t prev){
		size_t pad = Padding<L, T>::pad(prev);
		arr[N - sizeof...(Args) - 1] = prev + pad;
		prev += Size<L, T>::size(prev + pad);
		Offset<L, Args...>::fill_offsets(arr, prev);
	}
};
template<Layout L, typename T>
struct Offset<L, T> {
	static std::array<size_t, 1> offsets(){
		std::array<size_t, 1> arr;
		arr[0] = 0;
		return arr;
	}
	template<size_t N>
	static void fill_offsets(std::array<size_t, N> &arr, size_t prev){
		size_t pad = Padding<L, T>::pad(prev);
		arr[N - 1] = prev + pad;
		prev += Size<L, T>::size(prev + pad);
	}
};
}

#endif

