#include <iostream>
#include <vector>
#include <cassert>
#include <type_traits>
#include <typeinfo>

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
	//static const size_t size = sizeof(A) + sizeof(B);
	static void f(size_t prev = 0){
		prev += sizeof(T);
		std::cout << "Unpadded size=" << prev << ", ";
		prev = prev % alignof(T) == 0 ? prev
			: prev + alignof(T) - prev % alignof(T);
		std::cout << "Size with possible padding=" << prev << "\n\n";
		Size<List...>::f(prev);
	}
};
template<typename T>
struct Size<T> {
	static void f(size_t prev = 0){
		prev += sizeof(T);
		std::cout << "Unpadded size=" << prev << ", ";
		prev = prev % alignof(T) == 0 ? prev
			: prev + alignof(T) - prev % alignof(T);
		std::cout << "Size with padding=" << prev << "\n\n";
	}
};

template<int I, typename... List>
struct Offset;
template<int I, typename T, typename... List>
struct Offset<I, T, List...> {
	static_assert(I < sizeof...(List) + 1, "Offset index out of bounds");
	static void f(size_t prev = 0){
		prev += sizeof(T);
		std::cout << "Unpadded size=" << prev << ", ";
		prev = prev % alignof(T) == 0 ? prev
			: prev + alignof(T) - prev % alignof(T);
		std::cout << "Offset with possible padding=" << prev << "\n\n";
		Offset<I - 1, List...>::f(prev);
	}
};
template<typename T, typename... List>
struct Offset<0, T, List...> {
	static void f(size_t prev = 0){
		std::cout << "Unpadded size=" << prev << ", ";
		prev = prev % alignof(T) == 0 ? prev
			: prev + alignof(T) - prev % alignof(T);
		std::cout << "Offset with possible padding=" << prev << "\n\n";
		std::cout << "Offset of item=" << prev << "\n";
	}
};


/*
template<typename... Args>
class InterleavedArray {
	size_t sz;
	std::vector<char> data;

public:
	InterleavedArray(size_t size) : sz(size), data(sz * SizeOf<Args...>::value){}
	template<typename T>
	T& at(size_t i){
		assert(i < sz);
		static_assert(OffsetOf<T, Args...>::value != -1, "Type not in array");
		size_t offset = OffsetOf<T, Args...>::value;
		//Is there some better way to do this casting? Reinterpret cast might have alignment issues?
		T *t = static_cast<T*>(static_cast<void*>(&data[0] + offset + i * SizeOf<Args...>::value));
		return *t;
	}
	template<size_t I>
	typename TypeAt<I, Args...>::type& at(size_t i){
		assert(i < sz);
		using T = typename TypeAt<I, Args...>::type;
		static_assert(AlignedOffsetOfIndex<I, 1, T, Args...>::value != -1, "Type not in array");
		size_t offset = AlignedOffsetOfIndex<I, 1, T, Args...>::value;
		//Is there some better way to do this casting? Reinterpret cast might have alignment issues?
		T *t = static_cast<T*>(static_cast<void*>(&data[0] + offset + i * SizeOf<Args...>::value));
		return *t;
	}
	size_t size() const {
		return sz;
	}
};
*/

int main(int argc, char **argv){
	Size<double, char, double>::f();
	Offset<0, int16_t, double>::f();

	using TA = TypeAt<0, float, int, char>::type;
	using TB = TypeAt<1, float, int, char>::type;
	using TC = TypeAt<2, float, int, char>::type;
	static_assert(std::is_same<TA, float>::value && std::is_same<TB, int>::value
		&& std::is_same<TC, char>::value, "TypeAt failure");

	return 0;
}

