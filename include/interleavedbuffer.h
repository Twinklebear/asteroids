#ifndef INTERLEAVED_BUFFER_H
#define INTERLEAVED_BUFFER_H

#include <cassert>
#include <array>
#include <memory>
#include <tuple>
#include "gl_core_3_3.h"
#include "deleters.h"
#include "sequence.h"
#include "type_at.h"
#include "ptr_tuple.h"
#include "layout_size.h"
#include "layout_offset.h"

/*
 * A fixed capacity interleaved buffer stored on the device.
 * Stores an array of [Args, Args, ...] where Args will be commonly
 * referred to as a block. Layout mode can also be specified and will
 * control the placement of elements within blocks appropriately
 *
 * To store user defined structs in the STD140 layout you must pad
 * them yourself according the std140 rules to match with OpenGL
 * as there isn't much we can do there. For arrays of elements
 * that get padded (scalars, mat2, mat3) use a STD140Array
 */
template<Layout L, typename... Args>
class InterleavedBuffer {
	size_t capacity, stride_;
	std::shared_ptr<GLuint> buffer;
	GLenum mode, type, access;
	char *data;
	//Used for tracking where a mapped range begins and ends
	//if a range isn't mapped end is 0
	size_t map_start, map_end;
	std::array<size_t, sizeof...(Args)> offsets;

	using Size = detail::Size<L, Args...>;
	using Offset = detail::Offset<L, Args...>;

public:
	//tuple of pointers type returned by the tuple read function
	using PtrTuple = typename detail::PtrTuple<Args...>::type;
	/*
	 * Create a non-existant buffer, just provides an empty ctor to call
	 * so that you can defer buffer creation until initializing OpenGL
	 */
	InterleavedBuffer() : capacity(0), stride_(0), buffer(nullptr),
		mode(0), type(0), access(0), data(nullptr), map_start(0), map_end(0),
		offsets(Offset::offsets())
	{}
	/*
	 * Create an interleaved buffer with capable of storing capacity blocks of
	 * Args. The buffer will be of the type passed and use the desired access flag
	 */
	InterleavedBuffer(size_t capacity, GLenum type, GLenum access)
		: capacity(capacity), stride_(Size::size()), buffer(nullptr),
		mode(0), type(type), access(access), data(nullptr), map_start(0), map_end(0),
		offsets(Offset::offsets())
	{
		buffer = std::shared_ptr<GLuint>(new GLuint{0}, detail::delete_buffer);
		glGenBuffers(1, &(*buffer));
		glBindBuffer(type, *buffer);
		glBufferData(type, capacity * stride_, NULL, access);
	}
	~InterleavedBuffer(){
		//If they forgot to unmap the buffer
		if (data != nullptr){
			glBindBuffer(type, *buffer);
			glUnmapBuffer(type);
		}
	}
	/*
	 * Bind the buffer to the type target specified at creation
	 */
	void bind(){
		assert(buffer != nullptr);
		glBindBuffer(type, *buffer);
	}
	/*
	 * Bind the buffer to some other type target. This will not change
	 * the stored type of the buffer, just bind it to some other binding point
	 */
	void bind(GLenum target){
		assert(buffer != nullptr);
		glBindBuffer(target, *buffer);
	}
	/*
	 * Bind the entire buffer to the desired indexed buffer target
	 */
	void bind_base(int index){
		assert(buffer != nullptr);
		glBindBufferBase(type, index, *buffer);
	}
	/*
	 * Map the entire buffer for access with the desired mode, m
	 * The buffer must be mapped appropriately before calling any of
	 * read/write/at
	 */
	void map(GLenum m){
		bind();
		mode = m;
		map_start = 0;
		data = static_cast<char*>(glMapBuffer(type, mode));
	}
	/*
	 * Map a range of indices of the buffer for access with the desired mode, m
	 * The buffer must be mapped appropriately before calling any of
	 * read/write/at
	 */
	void map_range(size_t start, size_t length, int flags){
		assert(start < capacity && length > 0 && start + length <= capacity);
		bind();
		mode = flags;
		map_start = start;
		map_end = start + length;
		data = static_cast<char*>(glMapBufferRange(type, map_start * stride_,
			length * stride_, flags));
	}
	/*
	 * Flushes a range of the buffer starting at start. The buffer must be bound
	 * before calling this function
	 */
	void flush_range(size_t start, size_t length){
		assert(data != nullptr);
		assert(map_end > 0 && map_start <= start && start + length <= map_end
			&& (mode & GL_MAP_FLUSH_EXPLICIT_BIT));
		glFlushMappedBufferRange(type, start * stride_, length * stride_);
	}
	/*
	 * Unmap the buffer, it's assumed the buffer was mapped as the type set
	 * at creation.
	 * TODO: Provide another version that stores any changes to the type at bind/unbind?
	 * can a buffer be bound at multiple targets?
	 */
	void unmap(){
		mode = 0;
		data = nullptr;
		map_end = 0;
		bind();
		glUnmapBuffer(type);
	}
	/*
	 * Get a read-only reference to block member I at index i in the array
	 * The buffer must be at least mapped for reading with i in the
	 * mapped range
	 */
	template<size_t I>
	const typename detail::TypeAt<I, Args...>::type& read(size_t i) const {
		assert(data != nullptr);
		//Check appropriate assert for our map state
		if (map_end > 0){
			assert(i >= map_start && i < map_end && (mode & GL_MAP_READ_BIT));
		}
		else {
			assert(i < capacity && (mode == GL_READ_ONLY || mode == GL_READ_WRITE));
		}
		return get<I>(i);
	}
	/*
	 * Get a write-only reference to block member I at index i in the array
	 * The buffer must be at least mapped for writing with i in the
	 * mapped range
	 */
	template<size_t I>
	typename detail::TypeAt<I, Args...>::type& write(size_t i){
		assert(data != nullptr);
		//Check appopriate assert for our map state
		if (map_end > 0){
			assert(i >= map_start && i < map_end && (mode & GL_MAP_WRITE_BIT));
		}
		else {
			assert(i < capacity && (mode == GL_WRITE_ONLY || mode == GL_READ_WRITE));
		}
		return get<I>(i);
	}
	/*
	 * Get a read-write reference to block member I at index i in the array
	 * The buffer must be mapped for reading and writing with i in the
	 * mapped range
	 */
	template<size_t I>
	typename detail::TypeAt<I, Args...>::type& at(size_t i){
		assert(data != nullptr);
		//Check appopriate assert for our map state
		if (map_end > 0){
			assert(i >= map_start && i < map_end && (mode & (GL_MAP_WRITE_BIT | GL_MAP_READ_BIT)));
		}
		else {
			assert(i < capacity && mode == GL_READ_WRITE);
		}
		return get<I>(i);
	}
	/*
	 * Read a block of values from the buffer at index i
	 */
	PtrTuple at(size_t i){
		assert(data != nullptr);
		//Check appopriate assert for our map state
		if (map_end > 0){
			assert(i >= map_start && i < map_end && (mode & (GL_MAP_WRITE_BIT | GL_MAP_READ_BIT)));
		}
		else {
			assert(i < capacity && mode == GL_READ_WRITE);
		}
		PtrTuple t;
		at(i, t, typename detail::GenSequence<sizeof...(Args)>::seq{});
		return t;
	}
	/*
	 * Write a block of values to the buffer at index i
	 */
	void write(size_t i, const std::tuple<Args...> &args){
		assert(data != nullptr);
		//Check appopriate assert for our map state
		if (map_end > 0){
			assert(i >= map_start && i < map_end && (mode & GL_MAP_WRITE_BIT));
		}
		else {
			assert(i < capacity && (mode == GL_WRITE_ONLY || mode == GL_READ_WRITE));
		}
		write(i, args, typename detail::GenSequence<sizeof...(Args)>::seq{});
	}
	/*
	 * Get the number of blocks stored in the buffer
	 */
	size_t size() const {
		return capacity;
	}
	/*
	 * Get the stride in bytes between each block of elements in the buffer
	 */
	size_t stride() const {
		return stride_;
	}
	/*
	 * Get the offset of element i within a block
	 */
	size_t offset(size_t i) const {
		assert(i < sizeof...(Args));
		return offsets[i];
	}

private:
	/*
	 * Get a reference to block member I at index i
	 */
	template<size_t I>
	typename detail::TypeAt<I, Args...>::type& get(size_t i){
		using T = typename detail::TypeAt<I, Args...>::type;
		size_t offset = offsets[I];
		T *t = reinterpret_cast<T*>(data + offset + (i - map_start) * stride_);
		return *t;
	}
	template<size_t I>
	const typename detail::TypeAt<I, Args...>::type& get(size_t i) const {
		using T = typename detail::TypeAt<I, Args...>::type;
		size_t offset = offsets[I];
		const T *t = reinterpret_cast<const T*>(data + offset + (i - map_start) * stride_);
		return *t;
	}
	/*
	 * Recursively write tuple values into the block using the sequence to retrieve
	 * the tuple indices
	 */
	template<int N, int... S>
	void write(size_t i, const std::tuple<Args...> &args, detail::Sequence<N, S...>){
		get<N>(i) = std::get<N>(args);
		write(i, args, detail::Sequence<S...>{});
	}
	template<int N>
	void write(size_t i, const std::tuple<Args...> &args, detail::Sequence<N>){
		get<N>(i) = std::get<N>(args);
	}
	/*
	 * Recursively read values from the block into the tuple using the sequence to
	 * retrieve the indices
	 */
	template<int N, int... S>
	void at(size_t i, PtrTuple &t, detail::Sequence<N, S...>){
		std::get<N>(t) = &get<N>(i);
		at(i, t, detail::Sequence<S...>{});
	}
	template<int N>
	void at(size_t i, PtrTuple &t, detail::Sequence<N>){
		std::get<N>(t) = &get<N>(i);
	}
};

#endif

