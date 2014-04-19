#ifndef INTERLEAVED_BUFFER_H
#define INTERLEAVED_BUFFER_H

#include <cassert>
#include "gl_core_3_3.h"
#include "typeutils.h"

//A fixed capacity interleaved buffer stored on the device
template<typename... Args>
class InterleavedBuffer {
	size_t capacity, stride_;
	GLuint buffer;
	GLenum mode, type, access;
	char *data;

public:
	InterleavedBuffer() : capacity(capacity), stride_(0), buffer(0),
		mode(0), type(0), access(0), data(nullptr)
	{}
	InterleavedBuffer(size_t capacity, GLenum type, GLenum access)
		: capacity(capacity), stride_(detail::Size<Args...>::size()), buffer(0),
		mode(0), type(type), access(access), data(nullptr)
	{
		glGenBuffers(1, &buffer);
		glBindBuffer(type, buffer);
		glBufferData(type, capacity * stride_, NULL, access);
	}
	~InterleavedBuffer(){
		if (buffer != 0){
			glDeleteBuffers(1, &buffer);
		}
	}
	void bind(){
		glBindBuffer(type, buffer);
	}
	void map(GLenum m){
		bind();
		mode = m;
		data = static_cast<char*>(glMapBuffer(type, mode));
	}
	void unmap(){
		mode = 0;
		data = nullptr;
		glUnmapBuffer(type);
	}
	template<size_t I>
	const typename detail::TypeAt<I, Args...>::type& read(size_t i) const {
		assert(i < capacity && data != nullptr
			&& (mode == GL_READ_ONLY || mode == GL_READ_WRITE));
		using T = typename detail::TypeAt<I, Args...>::type;
		size_t offset = detail::Offset<I, Args...>::offset();
		T *t = static_cast<T*>(static_cast<void*>(data + offset + i * stride_));
		return *t;
	}
	template<size_t I>
	typename detail::TypeAt<I, Args...>::type& write(size_t i){
		assert(i < capacity && data != nullptr
			&& (mode == GL_WRITE_ONLY || mode == GL_READ_WRITE));
		using T = typename detail::TypeAt<I, Args...>::type;
		size_t offset = detail::Offset<I, Args...>::offset();
		T *t = static_cast<T*>(static_cast<void*>(data + offset + i * stride_));
		return *t;
	}
	template<size_t I>
	typename detail::TypeAt<I, Args...>::type& at(size_t i){
		assert(i < capacity && data != nullptr && mode == GL_READ_WRITE);
		using T = typename detail::TypeAt<I, Args...>::type;
		size_t offset = detail::Offset<I, Args...>::offset();
		T *t = static_cast<T*>(static_cast<void*>(data + offset + i * stride_));
		return *t;
	}
	size_t size() const {
		return capacity;
	}
	size_t stride() const {
		return stride_;
	}
};

#endif

