#ifndef TYPE_UTILS_H
#define TYPE_UTILS_H

#include <type_traits>
#include <typeinfo>

namespace detail {
template<int I, typename T, typename... List>
struct TypeAt {
	static_assert(I > -1 && I < sizeof...(List) + 1, "TypeAt index out of bounds");
	using type = typename TypeAt<I-1, List...>::type;
};
template<typename T, typename... List>
struct TypeAt<0, T, List...> {
	using type = T;
};

//How to handle std140 UBO layout? I guess a hack way to do it would just be
//to specialize for glm::vec3/mat4 if the sizeof/alignof isn't right for them
template<typename... List>
struct Size;
template<typename T, typename... List>
struct Size<T, List...> {
	static size_t size(size_t prev = 0){
		prev += sizeof(T);
		prev = prev % alignof(T) == 0 ? prev
			: prev + alignof(T) - prev % alignof(T);
		return Size<List...>::size(prev);
	}
};
template<typename T>
struct Size<T> {
	static size_t size(size_t prev = 0){
		prev += sizeof(T);
		prev = prev % alignof(T) == 0 ? prev
			: prev + alignof(T) - prev % alignof(T);
		return prev;
	}
};

template<int I, typename... List>
struct Offset;
template<int I, typename T, typename... List>
struct Offset<I, T, List...> {
	static_assert(I < sizeof...(List) + 1, "Offset index out of bounds");
	static size_t offset(size_t prev = 0){
		prev += sizeof(T);
		prev = prev % alignof(T) == 0 ? prev
			: prev + alignof(T) - prev % alignof(T);
		return Offset<I - 1, List...>::offset(prev);
	}
};
template<typename T, typename... List>
struct Offset<0, T, List...> {
	static size_t offset(size_t prev = 0){
		prev = prev % alignof(T) == 0 ? prev
			: prev + alignof(T) - prev % alignof(T);
		return prev;
	}
};
}

#endif

