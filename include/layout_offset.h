#ifndef BUFFER_OFFSET_H
#define BUFFER_OFFSET_H

#include "std140_array.h"
#include "layout_size.h"

namespace detail {
template<int I, Layout L, typename... Args>
struct Offset;
template<Layout L, typename... Args>
struct AllOffsets;
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
template<typename T, typename... Args>
struct AllOffsets<Layout::PACKED, T, Args...> {
	static std::array<size_t, 1 + sizeof...(Args)> offsets(){
		std::array<size_t, sizeof...(Args) + 1> arr;
		fill_offsets(arr, 0);
		return arr;
	}
	template<size_t N>
	static void fill_offsets(std::array<size_t, N> &arr, size_t prev){
		arr[N - sizeof...(Args) - 1] = prev;
		prev += sizeof(T);
		AllOffsets<Layout::PACKED, Args...>::fill_offsets(arr, prev);
	}
};
template<typename T>
struct AllOffsets<Layout::PACKED, T> {
	static std::array<size_t, 1> offsets(){
		std::array<size_t, 1> arr;
		arr[0] = 0;
		return arr;
	}
	template<size_t N>
	static void fill_offsets(std::array<size_t, N> &arr, size_t prev){
		arr[N - 1] = prev;
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
template<typename T, typename... Args>
struct AllOffsets<Layout::ALIGNED, T, Args...> {
	static std::array<size_t, 1 + sizeof...(Args)> offsets(){
		std::array<size_t, sizeof...(Args) + 1> arr;
		arr[0] = 0;
		AllOffsets<Layout::ALIGNED, Args...>::fill_offsets(arr, sizeof(T));
		return arr;
	}
	template<size_t N>
	static void fill_offsets(std::array<size_t, N> &arr, size_t prev){
		size_t pad = Padding<Layout::ALIGNED, T>::pad(prev);
		arr[N - sizeof...(Args) - 1] = prev + pad;
		std::cout << "Set offset for " << N - sizeof...(Args) - 1
			<< " to " << prev + pad << "\n";
		prev = Size<Layout::ALIGNED, T>::size(prev + pad);
		std::cout << "occupies until " << prev << "\n";
		AllOffsets<Layout::ALIGNED, Args...>::fill_offsets(arr, prev);
	}
};
template<typename T>
struct AllOffsets<Layout::ALIGNED, T> {
	static std::array<size_t, 1> offsets(){
		std::array<size_t, 1> arr;
		arr[0] = 0;
		return arr;
	}
	template<size_t N>
	static void fill_offsets(std::array<size_t, N> &arr, size_t prev){
		size_t pad = Padding<Layout::ALIGNED, T>::pad(prev);
		arr[N - 1] = prev + pad;
		std::cout << "Set offset for " << N - 1
			<< " to " << prev + pad << "\n";
		std::cout << "occupies until " << prev + pad + sizeof(T) << "\n";
	}
};
template<int I, typename T, typename... Args>
struct Offset<I, Layout::STD140, T, Args...> {
	static_assert(I < sizeof...(Args) + 1, "STD140Offset index out of bounds");
	static size_t offset(size_t prev = 0){
		//Repeating all the specializations for STD140 will be a huge pain.
		//should instead have something that returns the padding that should
		//be applied in front of the object given some alignment
		prev = Size<Layout::STD140, T>::size(prev);
		return Offset<I - 1, Layout::STD140, Args...>::offset(prev);
	}
};

template<typename T, typename... Args>
struct AllOffsets<Layout::STD140, T, Args...> {
	static std::array<size_t, 1 + sizeof...(Args)> offsets(){
		std::array<size_t, sizeof...(Args) + 1> arr;
		arr[0] = 0;
		AllOffsets<Layout::STD140, Args...>::fill_offsets(arr, sizeof(T));
		return arr;
	}
	template<size_t N>
	static void fill_offsets(std::array<size_t, N> &arr, size_t prev){
		size_t pad = Padding<Layout::STD140, T>::pad(prev);
		arr[N - sizeof...(Args) - 1] = prev + pad;
		std::cout << "Set offset for " << N - sizeof...(Args) - 1
			<< " to " << prev + pad << "\n";
		prev = Size<Layout::STD140, T>::size(prev + pad);
		std::cout << "occupies until " << prev << "\n";
		AllOffsets<Layout::STD140, Args...>::fill_offsets(arr, prev);
	}
};
template<typename T>
struct AllOffsets<Layout::STD140, T> {
	static std::array<size_t, 1> offsets(){
		std::array<size_t, 1> arr;
		arr[0] = 0;
		return arr;
	}
	template<size_t N>
	static void fill_offsets(std::array<size_t, N> &arr, size_t prev){
		size_t pad = Padding<Layout::STD140, T>::pad(prev);
		arr[N - 1] = prev + pad;
		std::cout << "Set offset for " << N - 1
			<< " to " << prev + pad << "\n";
		std::cout << "occupies until " << prev + pad + sizeof(T) << "\n";
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

