#include <iostream>
#include <cassert>
#include <type_traits>
#include <typeinfo>

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



//Get the total size in bytes of the arguments
template<typename T, typename... Args>
struct SizeOf {
	static const size_t value = sizeof(T) + SizeOf<Args...>::value;
};
template<typename T>
struct SizeOf<T> {
	static const size_t value = sizeof(T);
};
//Get the size in bytes of the arguments with each argument at the desired alignment
template<size_t Align, typename T, typename... Args>
struct AlignedSizeOf {
	static const size_t value = AlignedSizeOf<Align, Args...>::value
		+ (sizeof(T) % Align == 0 ? sizeof(T) : sizeof(T) + Align - sizeof(T) % Align);
};
template<size_t Align, typename T>
struct AlignedSizeOf<Align, T> {
	static const size_t value = sizeof(T) % Align == 0 ? sizeof(T) : sizeof(T) + Align - sizeof(T) % Align;
};

template<int I, typename T, typename... List>
struct TypeAt {
	static_assert(I > -1 && I < sizeof...(List) + 1, "TypeAt index out of bounds");
	using type = typename TypeAt<I-1, List...>::type;
};
template<typename T, typename... List>
struct TypeAt<0, T, List...> {
	using type = T;
};

//Find the offset by the index
template<int I, size_t Align, typename T, typename... List>
struct AlignedOffsetOfIndex {
	static_assert(I > -1 && I < sizeof...(List) + 1, "AlignedOffsetOfIndex index out of bounds");
	static const int64_t value = AlignedOffsetOfIndex<I-1, Align, List...>::value != -1 ?
		AlignedSizeOf<Align, T>::value + AlignedOffsetOfIndex<I-1, Align, List...>::value
		: -1;
};
template<size_t Align, typename T, typename... List>
struct AlignedOffsetOfIndex<0, Align, T, List...> {
	static const int64_t value = 0;
};
template<size_t Align, typename T>
struct AlignedOffsetOfIndex<0, Align, T> {
	static const int64_t value = 0;
};
//If I > 0 and there's only one item left in the list the index is out of bounds
template<int I, size_t Align, typename T>
struct AlignedOffsetOfIndex<I, Align, T> {
	//Should there be a static assert here?
	static const int64_t value = -1;
};

//Would also need indexed offset of to get offset by index instead of by type
template<size_t Align, typename T, typename A, typename... List>
struct AlignedOffsetOf;
template<bool same, size_t Align, typename T, typename A, typename... List>
struct AlignedOffsetOfHelper;
template<size_t Align, typename T, typename A, typename... List>
struct AlignedOffsetOfHelper<true, Align, T, A, List...> {
	static const int64_t value = 0;
};
template<size_t Align, typename T, typename A, typename... List>
struct AlignedOffsetOfHelper<false, Align, T, A, List...> {
	static const int64_t value = AlignedOffsetOf<Align, T, List...>::value != -1 ?
		AlignedSizeOf<Align, T>::value + AlignedOffsetOf<Align, T, List...>::value : -1;
};
template<size_t Align, typename T, typename A>
struct AlignedOffsetOfHelper<true, Align, T, A> {
	static const int64_t value = 0;
};
template<size_t Align, typename T, typename A>
struct AlignedOffsetOfHelper<false, Align, T, A> {
	static const int64_t value = -1;
};
//Get the offset in bytes of some type in the list
template<size_t Align, typename T, typename A, typename... List>
struct AlignedOffsetOf {
	static const int64_t value = AlignedOffsetOfHelper<std::is_same<T, A>::value, Align, T, A, List...>::value;
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
		<< "AlignedSizeOf<4, int, float, char>: " << AlignedSizeOf<4, int, float, char>::value << "\n"
		<< "AlignedSizeOf<4, int16_t, int16_t, int16_t>: "
		<< AlignedSizeOf<4, int16_t, int16_t, int16_t>::value << "\n";
	std::cout << "AlignedOffsetOf<4, float, int, char, float>: "
		<< AlignedOffsetOf<4, float, int, char, float>::value << "\n";
	std::cout << "AlignedOffsetOfIndex<1, int16_t, int16_t, int16_t>: "
		<< AlignedOffsetOfIndex<2, 4, int16_t, int16_t, int16_t>::value << "\n";

	using TA = TypeAt<0, float, int, char>::type;
	using TB = TypeAt<1, float, int, char>::type;
	using TC = TypeAt<2, float, int, char>::type;
	static_assert(std::is_same<TA, float>::value && std::is_same<TB, int>::value
		&& std::is_same<TC, char>::value, "TypeAt failure");

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

