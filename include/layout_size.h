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
template<Layout L, typename... Args>
struct Size;
template<typename T, typename... Args>
struct Size<Layout::PACKED, T, Args...> {
	static size_t size(size_t prev = 0){
		prev += sizeof(T);
		return Size<Layout::PACKED, Args...>::size(prev);
	}
};
template<typename T>
struct Size<Layout::PACKED, T> {
	static size_t size(size_t prev = 0){
		prev += sizeof(T);
		return prev;
	}
};
template<typename T, typename... Args>
struct Size<Layout::ALIGNED, T, Args...> {
	static size_t size(size_t prev = 0){
		prev += Padding<Layout::ALIGNED, T>::pad(prev) + sizeof(T);
		return Size<Layout::ALIGNED, Args...>::size(prev);
	}
};
template<typename T>
struct Size<Layout::ALIGNED, T> {
	static size_t size(size_t prev = 0){
		prev += Padding<Layout::ALIGNED, T>::pad(prev) + sizeof(T);
		return prev;
	}
};
template<typename T, typename... Args>
struct Size<Layout::STD140, T, Args...> {
	static size_t size(size_t prev = 0){
		//The padder takes care of applying proper rules for primitive types
		//such as scalars and vectors. If it's not a scalar, vector or
		//caught by our STD140Array or mat4 specializations we don't know
		//what to do, so give up
		prev += Padding<Layout::STD140, T>::pad(prev) + sizeof(T);
		return Size<Layout::STD140, Args...>::size(prev);
	}
};
template<typename T, size_t N, typename... Args>
struct Size<Layout::STD140, STD140Array<T, N>, Args...> {
	static size_t size(size_t prev = 0){
		//TODO: Couldn't we use our padded here too?
		//Rule 4 for arrays
		prev += N * STD140Array<T, N>::stride();
		//Rule also states we align as a vec4
		using V = glm::vec4::value_type;
		prev = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return Size<Layout::STD140, Args...>::size(prev);
	}
};
template<typename... Args>
struct Size<Layout::STD140, glm::mat4, Args...> {
	static size_t size(size_t prev = 0){
		//Rule 5/7 for mat4's (ie. treat as array)
		return Size<Layout::STD140,
		   STD140Array<glm::mat4, 1>, Args...>::size(prev);
	}
};
template<typename T>
struct Size<Layout::STD140, T> {
	static size_t size(size_t prev = 0){
		prev += Padding<Layout::STD140, T>::pad(prev) + sizeof(T);
		return prev;
	}
};
template<typename T, size_t N>
struct Size<Layout::STD140, STD140Array<T, N>> {
	static size_t size(size_t prev = 0){
		//Rule 4 for arrays
		prev += Padding<Layout::STD140, T>::pad(prev)
			+ N * STD140Array<T, N>::stride();;
		return prev;
	}
};
template<>
struct Size<Layout::STD140, glm::mat4> {
	static size_t size(size_t prev = 0){
		//Rule 5/7 for mat4's (ie. treat as array)
		return Size<Layout::STD140, STD140Array<glm::mat4, 1>>::size(prev);
	}
};
}

#endif

