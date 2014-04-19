#include <iostream>
#include <tuple>
#include <vector>
#include <glm/glm.hpp>
#include "gl_core_3_3.h"
#include "interleavedbuffer.h"
#include "renderbatch.h"

RenderBatch::RenderBatch(size_t capacity) : size(0), capacity(capacity),
	vao(0), matrices(capacity, GL_ARRAY_BUFFER, GL_STREAM_DRAW)
{
	glGenVertexArrays(1, &vao);
}
RenderBatch::~RenderBatch(){
	glDeleteVertexArrays(1, &vao);
}
void RenderBatch::push_back(const std::vector<glm::mat4> &objs){
	if (size + objs.size() > capacity){
		resize_buffer();
	}
	matrices.map(GL_WRITE_ONLY);
	for (size_t i = 0; i < objs.size(); ++i){
		matrices.write<0>(size + i) = objs[i];
	}
	matrices.unmap();
	size += objs.size();
}
void RenderBatch::push_back(const glm::mat4 &mat){
	if (size + 1 > capacity){
		resize_buffer();
	}
	matrices.map(GL_WRITE_ONLY);
	matrices.write<0>(size) = mat;
	matrices.unmap();
	size += 1;
}
void RenderBatch::update(const std::vector<std::tuple<size_t, glm::mat4>> &updates){
	matrices.map(GL_WRITE_ONLY);
	for (const std::tuple<size_t, glm::mat4> t : updates){
		size_t i = std::get<0>(t);
		assert(i < size);
		matrices.write<0>(i) = std::get<1>(t);
	}
	matrices.unmap();
}
void RenderBatch::pop_back(){
	--size;
}
void RenderBatch::remove(size_t i){
	assert(i < size);
	matrices.map(GL_WRITE_ONLY);
	for (size_t j = i; j < size - 1; ++j){
		matrices.write<0>(j) = matrices.write<0>(j + 1);
	}
	matrices.unmap();
	--size;
}
void RenderBatch::set_attrib_index(unsigned attrib){
	glBindVertexArray(vao);
	matrices.bind();
	for (unsigned i = attrib; i < attrib + 4; ++i){
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, matrices.stride(),
				(void*)(sizeof(glm::vec4) * i));
		glVertexAttribDivisor(i, 1);
	}
}
void RenderBatch::render(){
	glBindVertexArray(vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, size);
}
size_t RenderBatch::batch_size() const {
	return size;
}
void RenderBatch::resize_buffer(){
	std::cerr << "Performance warning: RenderBatch attribute buffer resizing, consider making it bigger\n";
	capacity *= 2;
	InterleavedBuffer<glm::mat4> new_mat(capacity, GL_ARRAY_BUFFER,	GL_STREAM_DRAW);
	new_mat.bind(GL_COPY_WRITE_BUFFER);
	matrices.bind(GL_COPY_READ_BUFFER);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
			size * detail::Size<glm::mat4>::size());
	matrices = new_mat;
}

