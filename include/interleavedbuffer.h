#ifndef INTERLEAVED_BUFFER_H
#define INTERLEAVED_BUFFER_H

#include <cassert>
#include <memory>
#include "gl_core_3_3.h"
#include "deleters.h"
#include "type_at.h"
#include "buffer_size.h"
#include "buffer_offset.h"

//A fixed capacity interleaved buffer stored on the device
template<Layout L, typename... Args>
class InterleavedBuffer {
	size_t capacity, stride_;
	std::shared_ptr<GLuint> buffer;
	GLenum mode, type, access;
	char *data;
	//Used for tracking where a mapped range begins and ends
	//if a range isn't mapped end is 0
	size_t map_start, map_end;

	using Size = detail::Size<L, Args...>;
	template<int I>
	using Offset = detail::Offset<I, L, Args...>;

public:
	InterleavedBuffer() : capacity(0), stride_(0), buffer(nullptr),
		mode(0), type(0), access(0), data(nullptr), map_start(0), map_end(0)
	{}
	InterleavedBuffer(size_t capacity, GLenum type, GLenum access)
		: capacity(capacity), stride_(Size::size()), buffer(nullptr),
		mode(0), type(type), access(access), data(nullptr), map_start(0), map_end(0)
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
	void bind(){
		glBindBuffer(type, *buffer);
	}
	void bind(GLenum target){
		glBindBuffer(target, *buffer);
	}
	void bind_base(int index){
		glBindBufferBase(type, index, *buffer);
	}
	void map(GLenum m){
		bind();
		mode = m;
		map_start = 0;
		data = static_cast<char*>(glMapBuffer(type, mode));
	}
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
	 * Flushes some range of the buffer starting at start. The buffer must be bound
	 * before calling this function
	 */
	void flush_range(size_t start, size_t length){
		assert(data != nullptr);
		assert(map_end > 0 && map_start <= start && start + length <= map_end
			&& (mode & GL_MAP_FLUSH_EXPLICIT_BIT));
		glFlushMappedBufferRange(type, start * stride_, length * stride_);
	}
	void unmap(){
		mode = 0;
		data = nullptr;
		map_end = 0;
		bind();
		glUnmapBuffer(type);
	}
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
		using T = typename detail::TypeAt<I, Args...>::type;
		size_t offset = Offset<I>::offset();
		T *t = reinterpret_cast<T*>(data + offset + (i - map_start) * stride_);
		return *t;
	}
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
	size_t size() const {
		return capacity;
	}
	size_t stride() const {
		return stride_;
	}

private:
	template<size_t I>
	typename detail::TypeAt<I, Args...>::type& get(size_t i){
		using T = typename detail::TypeAt<I, Args...>::type;
		size_t offset = Offset<I>::offset();
		T *t = reinterpret_cast<T*>(data + offset + (i - map_start) * stride_);
		return *t;
	}
};

#endif

