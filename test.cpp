#include <iostream>
#include <cassert>
#include <type_traits>

template<int N, int I = 0>
class For {
	enum { go = I < N };

public:
	static void f(){
		std::cout << "Loop " << I << "\n";
		For<N, go ? (I + 1) : N>::f();
	}
};
template<int N>
class For<N, N> {
public:
	static void f(){
		std::cout << "Loop end\n";
	}
};

template<long N>
struct binary {
	static const long value = binary<N/10>::value << 1 | N % 10;
};
template<>
struct binary<0> {
	static const long value = 0;
};

//Get the total size in bytes of the arguments
template<typename T, typename... Args>
struct SizeOf {
	static const size_t value = sizeof(T) + SizeOf<Args...>::value;
};
template<typename T>
struct SizeOf<T> {
	static const size_t value = sizeof(T);
};
//Get the size in bytes of the arguments padded out to the nearest multiple of the alignment
template<size_t Align, typename T, typename... Args>
struct AlignedSizeOf {
	static const size_t value = (sizeof(T) + SizeOf<Args...>::value) % Align == 0 ?
		sizeof(T) + SizeOf<Args...>::value
		: sizeof(T) + SizeOf<Args...>::value + Align - (sizeof(T) + SizeOf<Args...>::value) % Align;
};

template<typename T, typename A, typename... List>
struct OffsetOf;
template<bool same, typename T, typename A, typename... List>
struct OffsetOfHelper;
template<typename T, typename A, typename... List>
struct OffsetOfHelper<true, T, A, List...> {
	static const int64_t value = 0;
};
template<typename T, typename A, typename... List>
struct OffsetOfHelper<false, T, A, List...> {
	static const int64_t value = OffsetOf<T, List...>::value != -1 ? sizeof(A) + OffsetOf<T, List...>::value : -1;
};
template<typename T, typename A>
struct OffsetOfHelper<true, T, A> {
	static const int64_t value = 0;
};
template<typename T, typename A>
struct OffsetOfHelper<false, T, A> {
	static const int64_t value = -1;
};
//Get the offset in bytes of some type in the list
template<typename T, typename A, typename... List>
struct OffsetOf {
	static const int64_t value = OffsetOfHelper<std::is_same<T, A>::value, T, A, List...>::value;
};

template<typename... Args>
class InterleavedArray {
	size_t sz;
	char *data;

public:
	InterleavedArray(size_t size) : sz(size), data(new char[sz * SizeOf<Args...>::value]){}
	~InterleavedArray(){
		delete[] data;
	}
	template<typename T>
	T& at(size_t i){
		assert(i < sz);
		static_assert(OffsetOf<T, Args...>::value != -1, "Type not in array");
		size_t offset = OffsetOf<T, Args...>::value;
		T *t = static_cast<T*>(static_cast<void*>(data + offset + i * SizeOf<Args...>::value));
		return *t;
	}
	size_t size() const {
		return sz;
	}
};

int main(int argc, char **argv){
	std::cout << "char: " << sizeof(char) << ", size % 4=" << sizeof(char) % 4 << "\n"
		<< "int: " << sizeof(int) << ", size % 4=" << sizeof(int) % 4 << "\n";
	std::cout << "SizeOf<int, float, char>: " << SizeOf<int, float, char>::value << "\n"
		<< "AlignedSizeOf<int, float, char>: " << AlignedSizeOf<4, int, float, char>::value << "\n"
		<< "AlignedSizeOf<int16_t, int16_t, int16_t>: "
		<< AlignedSizeOf<4, int16_t, int16_t, int16_t>::value << "\n";

	InterleavedArray<int, float, char> array(2);
	array.at<int>(0) = 10;
	array.at<int>(1) = 20;
	array.at<float>(0) = 15.f;
	array.at<float>(1) = 5.f;
	array.at<char>(0) = 'A';
	array.at<char>(1) = 'B';
	for (size_t i = 0; i < array.size(); ++i){
		std::cout << "Data at " << i
			<< ": int=" << array.at<int>(i)
			<< ", float=" << array.at<float>(i)
			<< ", char=" << array.at<char>(i) << "\n";
	}
	return 0;
}

