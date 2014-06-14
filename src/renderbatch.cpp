#include <iostream>
#include <tuple>
#include <vector>
#include <glm/glm.hpp>
#include "gl_core_3_3.h"
#include "glattrib_type.h"
#include "interleavedbuffer.h"
#include "renderbatch.h"
#include "model.h"

RenderBatch::RenderBatch(size_t capacity, Model model) : size(0),
	model(model), matrices(capacity, GL_ARRAY_BUFFER, GL_STREAM_DRAW), attrib_idx(-1)
{}
void RenderBatch::push_back(const std::vector<glm::mat4> &objs){
	if (size + objs.size() > matrices.size()){
		resize_buffer(2 * matrices.size());
	}
	matrices.map(GL_WRITE_ONLY);
	for (size_t i = 0; i < objs.size(); ++i){
		matrices.write<0>(size + i) = objs[i];
	}
	matrices.unmap();
	size += objs.size();
}
void RenderBatch::push_back(const glm::mat4 &mat){
	if (size + 1 > matrices.size()){
		resize_buffer(2 * matrices.size());
	}
	matrices.map(GL_WRITE_ONLY);
	matrices.write<0>(size) = mat;
	matrices.unmap();
	size += 1;
}
void RenderBatch::update(const std::vector<std::tuple<size_t, glm::mat4>> &updates){
	//Should we instead use FLUSH_EXPLICIT here and then condense the flushes into
	//blocks of the updated ranges of matrices?
	matrices.map(GL_WRITE_ONLY);
	for (const std::tuple<size_t, glm::mat4> &t : updates){
		size_t i = std::get<0>(t);
		assert(i < size);
		matrices.write(i, std::make_tuple(std::get<1>(t)));
	}
	matrices.unmap();
}
void RenderBatch::update(const std::vector<glm::mat4> &updates){
	assert(updates.size() <= size);
	matrices.map_range(0, updates.size(), GL_MAP_WRITE_BIT);
	for (size_t i = 0; i < updates.size(); ++i){
		matrices.write<0>(i) = updates[i];
	}
	matrices.unmap();
}
void RenderBatch::update(size_t i, const glm::mat4 &mat){
	matrices.map_range(i, 1, GL_MAP_WRITE_BIT);
	matrices.write<0>(i) = mat;
	matrices.unmap();
}
void RenderBatch::resize(size_t n){
	if (n > matrices.size()){
		resize_buffer(n);
	}
	size = n;
}
void RenderBatch::pop_back(size_t n){
	size -= n;
}
void RenderBatch::remove(size_t i){
	assert(i < size);
	matrices.map(GL_WRITE_ONLY);
	for (size_t j = i; j < size - 1; ++j){
		matrices.write(j, matrices.read(j + 1));
	}
	matrices.unmap();
	--size;
}
void RenderBatch::set_attrib_index(unsigned attrib){
	attrib_idx = attrib;
	model.bind();
	matrices.bind();
	//We know this will be 0 but it's more of a basic implementation of moving
	//to the automated setting
	size_t base_offset = matrices.offset(0);
	GLenum gl_type = detail::gl_attrib_type<glm::mat4>();
	//number of occupied indices is rounded based on vec4
	//would we want to use the Sizer for this?
	size_t attrib_size = detail::Size<Layout::PACKED, glm::mat4>::size();
	size_t num_indices = attrib_size / sizeof(glm::vec4);
	num_indices = attrib_size % sizeof(glm::vec4) == 0 ? num_indices : num_indices + 1;
	for (size_t i = 0; i < num_indices; ++i){
		glEnableVertexAttribArray(i + attrib);
		std::cout << "glVertexAttribPointer:\n"
			<< "  index: " << i + attrib
			<< "\n  raw offset: " << base_offset
			<< "\n  additional offset: " << i * sizeof(glm::vec4)
			<< "\n";
		if (gl_type == GL_FLOAT || gl_type == GL_HALF_FLOAT || gl_type == GL_DOUBLE){
			glVertexAttribPointer(i + attrib, 4, gl_type, GL_FALSE, matrices.stride(),
				(void*)(sizeof(glm::vec4) * i));
		}
		else {
			glVertexAttribIPointer(i + attrib, 4, gl_type, matrices.stride(),
				(void*)(sizeof(glm::vec4) * i));
		}
		glVertexAttribDivisor(i + attrib, 1);
	}
}
void RenderBatch::render(){
	model.bind();
	glDrawElementsInstanced(GL_TRIANGLES, model.elems(), GL_UNSIGNED_SHORT, 0, size);
}
size_t RenderBatch::batch_size() const {
	return size;
}
void RenderBatch::resize_buffer(size_t n){
	std::cerr << "Performance warning: RenderBatch attribute buffer resizing, consider making it bigger\n";
	InterleavedBuffer<Layout::PACKED, glm::mat4> new_mat(n, GL_ARRAY_BUFFER,
		GL_STREAM_DRAW);
	new_mat.bind(GL_COPY_WRITE_BUFFER);
	matrices.bind(GL_COPY_READ_BUFFER);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
			size * detail::Size<Layout::PACKED, glm::mat4>::size());
	matrices = new_mat;
	//If the attribute index has been set update the vao with the new buffer
	if (attrib_idx > -1){
		set_attrib_index(attrib_idx);
	}
}

