#ifndef BUFFER_SIZE_H
#define BUFFER_SIZE_H

#include <glm/glm.hpp>

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
		//Rule 4 for arrays. Arrays are assumed to be 1D
		//Note: we wouldn't be able to handle writing to arrays
		//though since we wouldn't have the right offsets for the indices
		if (std::is_array<T>::value){
			//using E = typename std::remove_all_extents<T>::type;
		}
		//Rule 1 for scalar alignment, also applied to unknown types
		else {
			prev += sizeof(T);
			prev = prev % sizeof(T) == 0 ? prev
				: prev + sizeof(T) - prev % sizeof(T);
		}
		return Size<Layout::ALIGNED, Args...>::size(prev);
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
		return Size<Layout::ALIGNED, Args...>::size(prev);
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
		return Size<Layout::ALIGNED, Args...>::size(prev);
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
		return Size<Layout::ALIGNED, Args...>::size(prev);
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
}

#endif

