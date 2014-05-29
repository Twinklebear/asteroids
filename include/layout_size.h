#ifndef BUFFER_SIZE_H
#define BUFFER_SIZE_H

#include <glm/glm.hpp>
#include "std140_array.h"
#include "layout_padding.h"

/*
 * Note: only rules 1-8 of the STD140 layout spec are implemented
 * as it's not possible (to my knowledge) to inspect struct members
 * and adjust the padding of structs. If you design your structs
 * so that they'll follow the layout rules they should be ok though
 * These layout rules are described here:
 * https://www.opengl.org/registry/specs/ARB/uniform_buffer_object.txt
 */
namespace detail {
template<Layout L, typename T, typename... Args>
struct Size {
	static size_t size(size_t prev = 0){
		prev += Padding<L, T>::pad(prev) + sizeof(T);
		return Size<L, Args...>::size(prev);
	}
};
template<Layout L, typename T>
struct Size<L, T> {
	static size_t size(size_t prev = 0){
		prev += Padding<L, T>::pad(prev) + sizeof(T);
		return prev;
	}
};
template<typename T, size_t N, typename... Args>
struct Size<Layout::STD140, STD140Array<T, N>, Args...> {
	static size_t size(size_t prev = 0){
		//TODO: Couldn't we use our padded here too?
		//Rule 4 for arrays
		prev += Padding<Layout::STD140, STD140Array<T, N>>::pad(prev)
			+ N * STD140Array<T, N>::stride();
		return Size<Layout::STD140, Args...>::size(prev);
	}
};
template<typename T, size_t N>
struct Size<Layout::STD140, STD140Array<T, N>> {
	static size_t size(size_t prev = 0){
		//Rule 4 for arrays
		prev += Padding<Layout::STD140, STD140Array<T, N>>::pad(prev)
			+ N * STD140Array<T, N>::stride();
		return prev;
	}
};
}

#endif

