#ifndef BUFFER_SIZE_H
#define BUFFER_SIZE_H

#include <glm/glm.hpp>
#include "std140_array.h"

/*
 * Note: only rules 1-8 of the STD140 layout spec are implemented
 * as it's not possible (to my knowledge) to inspect struct members
 * and adjust the padding of structs. If you design your structs
 * so that they'll follow the layout rules they should be ok though
 * These layout rules are described here:
 * https://www.opengl.org/registry/specs/ARB/uniform_buffer_object.txt
 */
enum class Layout { PACKED, ALIGNED, STD140 };
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
		prev += sizeof(T);
		prev = prev % alignof(T) == 0 ? prev
			: prev + alignof(T) - prev % alignof(T);
		return Size<Layout::ALIGNED, Args...>::size(prev);
	}
};
template<typename T>
struct Size<Layout::ALIGNED, T> {
	static size_t size(size_t prev = 0){
		prev += sizeof(T);
		prev = prev % alignof(T) == 0 ? prev
			: prev + alignof(T) - prev % alignof(T);
		return prev;
	}
};
template<typename T, typename... Args>
struct Size<Layout::STD140, T, Args...> {
	static size_t size(size_t prev = 0){
		//Rule 1 for scalar alignment, also applied to unknown types
		prev += sizeof(T);
		prev = prev % sizeof(T) == 0 ? prev
			: prev + sizeof(T) - prev % sizeof(T);
		return Size<Layout::STD140, Args...>::size(prev);
	}
};
template<typename... Args>
struct Size<Layout::STD140, glm::vec2, Args...> {
	static size_t size(size_t prev = 0){
		prev += sizeof(glm::vec2);
		//Rule 2 for 2 component vector
		using V = glm::vec2::value_type;
		prev = prev % (2 * sizeof(V)) == 0 ? prev
			: prev + 2 * sizeof(V) - prev % (2 * sizeof(V));
		return Size<Layout::STD140, Args...>::size(prev);
	}
};
template<typename... Args>
struct Size<Layout::STD140, glm::vec3, Args...> {
	static size_t size(size_t prev = 0){
		prev += sizeof(glm::vec3);
		//Rule 3 for 3 component vector
		using V = glm::vec3::value_type;
		prev = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return Size<Layout::STD140, Args...>::size(prev);
	}
};
template<typename... Args>
struct Size<Layout::STD140, glm::vec4, Args...> {
	static size_t size(size_t prev = 0){
		prev += sizeof(glm::vec4);
		//Rule 2 for 4 component vector
		using V = glm::vec4::value_type;
		prev = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return Size<Layout::STD140, Args...>::size(prev);
	}
};
template<typename T, size_t N, typename... Args>
struct Size<Layout::STD140, STD140Array<T, N>, Args...> {
	static size_t size(size_t prev = 0){
		//Rule 4 for arrays
		prev += N * STD140Array<T, N>::stride();
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
		prev += sizeof(T);
		//Rule 1 for scalar alignment
		if (std::is_arithmetic<T>::value){
			prev = prev % sizeof(T) == 0 ? prev
				: prev + sizeof(T) - prev % sizeof(T);
		}
		return prev;
	}
};
template<>
struct Size<Layout::STD140, glm::vec2> {
	static size_t size(size_t prev = 0){
		prev += sizeof(glm::vec2);
		//Rule 2 for 2 component vector
		using V = glm::vec2::value_type;
		prev = prev % (2 * sizeof(V)) == 0 ? prev
			: prev + 2 * sizeof(V) - prev % (2 * sizeof(V));
		return prev;
	}
};
template<>
struct Size<Layout::STD140, glm::vec3> {
	static size_t size(size_t prev = 0){
		prev += sizeof(glm::vec3);
		//Rule 3 for 3 component vector
		using V = glm::vec3::value_type;
		prev = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return prev;
	}
};
template<>
struct Size<Layout::STD140, glm::vec4> {
	static size_t size(size_t prev = 0){
		prev += sizeof(glm::vec4);
		//Rule 2 for 4 component vector
		using V = glm::vec4::value_type;
		prev = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return prev;
	}
};
template<typename T, size_t N>
struct Size<Layout::STD140, STD140Array<T, N>> {
	static size_t size(size_t prev = 0){
		//Rule 4 for arrays
		prev += N * STD140Array<T, N>::stride();
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

