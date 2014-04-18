#include <iostream>
#include <vector>
#include <cassert>
#include <type_traits>
#include <typeinfo>

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

template<typename... List>
struct TestSize;
template<typename A, typename B, typename... List>
struct TestSize<A, B, List...> {
	//static const size_t size = sizeof(A) + sizeof(B);
	static void f(size_t prev = 0){
		std::cout << "A = '" << typeid(A).name() << "', B = '" << typeid(B).name() << "'\n"
			<< "sizeof(A)=" << sizeof(A) << ", alignof(B)=" << alignof(B) << "\n"
			<< "prev size=" << prev << "\n";
		prev += sizeof(A);
		std::cout << "Unpadded size=" << prev << ", ";
		prev = prev % alignof(B) == 0 ? prev
			: prev + alignof(B) - prev % alignof(B);
		std::cout << "Size with possible padding=" << prev << "\n\n";
		TestSize<B, List...>::f(prev);
	}
};
template<typename A, typename B>
struct TestSize<A, B> {
	static void f(size_t prev = 0){
		std::cout << "A = '" << typeid(A).name() << "', B = '" << typeid(B).name() << "'\n"
			<< "sizeof(A)=" << sizeof(A) << ", alignof(B)=" << alignof(B) << "\n"
			<< "prev size=" << prev << "\n";
		prev += sizeof(A);
		std::cout << "Unpadded size=" << prev << ", ";
		prev = prev % alignof(B) == 0 ? prev
			: prev + alignof(B) - prev % alignof(B);
		std::cout << "Size with padding=" << prev << "\n\n";
		TestSize<B>::f(prev);
	}
};
template<typename A>
struct TestSize<A> {
	static void f(size_t prev = 0){
		std::cout << "A = '" << typeid(A).name() << "'\n"
			<< "Final size=" << prev + sizeof(A) << "\n\n";
	}
};

template<int I, typename... List>
struct TestOffset;
template<int I, typename A, typename B, typename... List>
struct TestOffset<I, A, B, List...> {
	//static const size_t size = sizeof(A) + sizeof(B);
	static void f(size_t prev = 0){
		std::cout << "A = '" << typeid(A).name() << "', B = '" << typeid(B).name() << "'\n"
			<< "sizeof(A)=" << sizeof(A) << ", alignof(B)=" << alignof(B) << "\n"
			<< "prev size=" << prev << "\n";
		prev += sizeof(A);
		std::cout << "Unpadded size=" << prev << ", ";
		prev = prev % alignof(B) == 0 ? prev
			: prev + alignof(B) - prev % alignof(B);
		std::cout << "Offset with possible padding=" << prev << "\n\n";
		TestOffset<I - 1, B, List...>::f(prev);
	}
};
template<int I, typename A, typename B>
struct TestOffset<I, A, B> {
	static void f(size_t prev = 0){
		std::cout << "A = '" << typeid(A).name() << "', B = '" << typeid(B).name() << "'\n"
			<< "sizeof(A)=" << sizeof(A) << ", alignof(B)=" << alignof(B) << "\n"
			<< "prev size=" << prev << "\n";
		prev += sizeof(A);
		std::cout << "Unpadded size=" << prev << ", ";
		prev = prev % alignof(B) == 0 ? prev
			: prev + alignof(B) - prev % alignof(B);
		std::cout << "Offset with padding=" << prev << "\n\n";
		TestOffset<I - 1, B>::f(prev);
	}
};
template<typename... List>
struct TestOffset<0, List...> {
	static void f(size_t prev = 0){
		std::cout << "Offset of item=" << prev << "\n";
	}
};


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

int main(int argc, char **argv){
	//This doesn't work for getting the 0th offset
	TestOffset<1, int, double>::f();

	using TA = TypeAt<0, float, int, char>::type;
	using TB = TypeAt<1, float, int, char>::type;
	using TC = TypeAt<2, float, int, char>::type;
	static_assert(std::is_same<TA, float>::value && std::is_same<TB, int>::value
		&& std::is_same<TC, char>::value, "TypeAt failure");

	return 0;
}

