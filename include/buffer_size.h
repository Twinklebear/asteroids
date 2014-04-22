#ifndef BUFFER_SIZE_H
#define BUFFER_SIZE_H

//TODO STD140 seems like it would require lots of special cases to handle
//the alignment
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
}

#endif

