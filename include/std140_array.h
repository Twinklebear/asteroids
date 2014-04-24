#ifndef STD140_ARRAY_H
#define STD140_ARRAY_H

#include <array>
#include <type_traits>
#include <glm/glm.hpp>

//This won't work for matrices
template<typename T, size_t N>
class STD140Array {
	static_assert(!std::is_array<T>::value, "Multidimensional arrays not supported");
	//std::array vs. std::vector? These arrays could be pretty big if they've got lots of
	//matrices or something. Although how many will I really be sending?
	std::vector<char> data;

public:
	STD140Array() : data(N * stride()) {}
	T& operator[](size_t i){
		assert(i < N);
		T *t = reinterpret_cast<T*>(&data[0] + stride() * i);
		return *t;
	}
	const T& operator[](size_t i) const {
		assert(i < N);
		T *t = reinterpret_cast<T*>(&data[0] + stride() * i);
		return *t;
	}
	T read(size_t i){
		return operator[](i);
	}
	const T read(size_t i) const {
		return operator[](i);
	}
	void write(size_t i, const T &t){
		operator[](i) = t;
	}
	size_t size(){
		return N;
	}
	/*
	 * Compute the array stride for the std140 layout requirements
	 * which requires that elements are aligned to vec4 (16 byte) alignment
	 */
	constexpr size_t stride(){
		return sizeof(T) % 16 == 0 ? sizeof(T) : sizeof(T) + 16 - sizeof(T) % 16;
	}
};
/*
 * The std140 matrices are an bit of a pain since they're passed as arrays
 * and so they get the padding applied to them as well, but GLM doesn't do this
 * so we need to handle converting between the padded and unpadded storage
 */
template<size_t N>
class STD140Array<glm::mat2, N> {
	STD140Array<glm::vec2, 2 * N> array;

public:
	STD140Array() : array() {}
	glm::mat2 read(size_t i){
		glm::mat2 m;
		m[0] = array[2 * i];
		m[1] = array[2 * i + 1];
		return m;
	}
	const glm::mat2 read(size_t i) const {
		glm::mat2 m;
		m[0] = array[2 * i];
		m[1] = array[2 * i + 1];
		return m;
	}
	void write(size_t i, const glm::mat2 &m){
		array[2 * i] = m[0];
		array[2 * i + 1] = m[1];
	}
	size_t size(){
		return N;
	}
	constexpr size_t stride(){
		return 2 * array.stride();
	}
};
template<size_t N>
class STD140Array<glm::mat3, N> {
	STD140Array<glm::vec3, 3 * N> array;

public:
	STD140Array() : array() {}
	glm::mat3 read(size_t i){
		glm::mat3 m;
		m[0] = array[3 * i];
		m[1] = array[3 * i + 1];
		m[2] = array[3 * i + 2];
		return m;
	}
	const glm::mat3 read(size_t i) const {
		glm::mat3 m;
		m[0] = array[3 * i];
		m[1] = array[3 * i + 1];
		m[2] = array[3 * i + 2];
		return m;
	}
	void write(size_t i, const glm::mat3 &m){
		array[3 * i] = m[0];
		array[3 * i + 1] = m[1];
		array[3 * i + 2] = m[2];
	}
	size_t size(){
		return N;
	}
	constexpr size_t stride(){
		return 3 * array.stride();
	}
};

#endif

