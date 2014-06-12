#ifndef LAYOUT_PADDING_H
#define LAYOUT_PADDING_H

#include "std140_array.h"

/*
 * Padding computes the number of bytes of padding to be placed
 * before some type T in a buffer where the previous object ends
 * at prev
 */
enum class Layout { PACKED, STD140 };
namespace detail {
template<Layout L, typename T>
struct Padding;
/*
 * Packed layout has no padding applied
 */
template<typename T>
struct Padding<Layout::PACKED, T> {
	static constexpr size_t pad(size_t){
		return 0;
	}
};
/*
 * STD140 Layout follows the rules described for STD140 buffer
 * layout, however for full compliance with the rules STD140Arrays
 * must be used instead of regular arrays
 */
template<typename T>
struct Padding<Layout::STD140, T> {
	static_assert(!std::is_array<T>::value, "Must use STD140Array for arrays in STD140");
	static size_t pad(size_t prev = 0){
		//Rule 1 for scalar alignment. If it's not a scalar and
		//not caught by our specializations we just kind of give up
		//and pretend it is a scalar
		size_t padded = prev % sizeof(T) == 0 ? prev
			: prev + sizeof(T) - prev % sizeof(T);
		return padded - prev;
	}
};
template<>
struct Padding<Layout::STD140, glm::vec2> {
	static size_t pad(size_t prev = 0){
		//Rule 2 for 2 component vector
		using V = glm::vec2::value_type;
		size_t padded = prev % (2 * sizeof(V)) == 0 ? prev
			: prev + 2 * sizeof(V) - prev % (2 * sizeof(V));
		return padded - prev;
	}
};
template<>
struct Padding<Layout::STD140, glm::vec3> {
	static size_t pad(size_t prev = 0){
		//Rule 3 for 3 component vector
		using V = glm::vec3::value_type;
		size_t padded = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return padded - prev;
	}
};
template<>
struct Padding<Layout::STD140, glm::vec4> {
	static size_t pad(size_t prev = 0){
		//Rule 2 for 4 component vector
		using V = glm::vec4::value_type;
		size_t padded = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return padded - prev;
	}
};
template<typename T, size_t N>
struct Padding<Layout::STD140, STD140Array<T, N>> {
	static size_t pad(size_t prev = 0){
		//Rule 4 for arrays (align to vec4)
		using V = glm::vec4::value_type;
		size_t padded = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return padded - prev;
	}
};
template<>
struct Padding<Layout::STD140, glm::mat4> {
	static size_t pad(size_t prev = 0){
		//Rule 5/7 for matrices (align to vec4)
		using V = glm::mat4::value_type;
		size_t padded = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return padded - prev;
	}
};
}

#endif

