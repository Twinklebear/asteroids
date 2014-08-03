#ifndef INTERLEAVED_TEX_BUFFER_H
#define INTERLEAVED_TEX_BUFFER_h

#include <cassert>
#include <array>
#include <memory>
#include <tuple>
#include "gl_core_3_3.h"
#include "deleters.h"

/*
 * A fixed capacity interleaved texture buffer stored on the device.
 * Stores an array of [Args, Args, ...] where Args will be commonly
 * referred to as a block. Layout mode for Texture Buffers will
 * always be packed
 *
 * You should also take care that the image format you specify
 * is sensible for the data you're planning to write to the buffer
 */
template<typename... Args>
class InterleavedTexBuffer {
	std::shared_ptr<GLuint> texture;
	GLenum format;
	InterleavedBuffer<Layout::PACKED, Args...> buffer;

public:
	/*
	 * Create a texture buffer and set the buffer containing the texture buffer's data
	 */
	InterleavedTexBuffer(GLenum format,	InterleavedBuffer<Layout::PACKED, Args...> &buffer)
		: format(format), buffer(buffer)
	{
		texture = std::shared_ptr<GLuint>(new GLuint{0}, detail::delete_texture);
		glGenTextures(1, &(*texture));
		glBindTexture(GL_TEXTURE_BUFFER, *texture);
		glTexBuffer(GL_TEXTURE_BUFFER, format, buffer.buf());
	}
	/*
	 * Bind the texture buffer
	 */
	void bind(){
		glBindTexture(GL_TEXTURE_BUFFER, *texture);
	}
	/*
	 * Get access to the buffer providing the texture buffer's data
	 */
	InterleavedBuffer<Layout::PACKED, Args...>& buf(){
		return buffer;
	}
	/*
	 * Associate a new buffer with the existing texture buffer
	 */
	void set_buffer(InterleavedBuffer<Layout::PACKED, Args...> &buf){
		buffer = buf;
		bind();
		glTexBuffer(GL_TEXTURE_BUFFER, format, buffer.buf());
	}
};

#endif

