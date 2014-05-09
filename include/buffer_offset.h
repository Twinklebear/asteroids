#ifndef BUFFER_OFFSET_H
#define BUFFER_OFFSET_H

#include "std140_array.h"
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
template<int I, typename T, typename... Args>
struct Offset<I, Layout::STD140, T, Args...> {
	static_assert(I < sizeof...(Args) + 1, "STD140Offset index out of bounds");
	static size_t offset(size_t prev = 0){
		prev = Size<Layout::STD140, T>::size(prev);
		return Offset<I - 1, Layout::STD140, Args...>::offset(prev);
	}
};
template<typename T, typename... Args>
struct Offset<0, Layout::STD140, T, Args...> {
	static size_t offset(size_t prev = 0){
		//Rule 1 for scalars
		if (std::is_arithmetic<T>::value){
			prev = prev % alignof(T) == 0 ? prev
				: prev + alignof(T) - prev % alignof(T);
		}
		return prev;
	}
};
template<typename... Args>
struct Offset<0, Layout::STD140, glm::vec2, Args...> {
	static size_t offset(size_t prev = 0){
		//Rule 2 for 2 component vector
		using V = glm::vec2::value_type;
		prev = prev % (2 * sizeof(V)) == 0 ? prev
			: prev + 2 * sizeof(V) - prev % (2 * sizeof(V));
		return prev;
	}
};
template<typename... Args>
struct Offset<0, Layout::STD140, glm::vec3, Args...> {
	static size_t offset(size_t prev = 0){
		//Rule 3 for 3 component vector
		using V = glm::vec3::value_type;
		prev = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return prev;
	}
};
template<typename... Args>
struct Offset<0, Layout::STD140, glm::vec4, Args...> {
	static size_t offset(size_t prev = 0){
		//Rule 2 for 4 component vector
		using V = glm::vec4::value_type;
		prev = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return prev;
	}
};
template<typename T, size_t N, typename... Args>
struct Offset<0, Layout::STD140, STD140Array<T, N>, Args...> {
	static size_t offset(size_t prev = 0){
		//Rule 4 for arrays (align to vec4)
		using V = glm::vec4::value_type;
		prev = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return prev;
	}
};
template<typename... Args>
struct Offset<0, Layout::STD140, glm::mat4, Args...> {
	static size_t offset(size_t prev = 0){
		//Rule 5/7 for matrices (align to vec4)
		using V = glm::mat4::value_type;
		prev = prev % (4 * sizeof(V)) == 0 ? prev
			: prev + 4 * sizeof(V) - prev % (4 * sizeof(V));
		return prev;
	}
};
}

#endif

