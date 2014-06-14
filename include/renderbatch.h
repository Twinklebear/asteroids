#ifndef RENDER_BATCH_H
#define RENDER_BATCH_H

#include <iostream>
#include <tuple>
#include <vector>
#include <glm/glm.hpp>
#include "gl_core_3_3.h"
#include "glattrib_type.h"
#include "interleavedbuffer.h"
#include "renderbatch.h"
#include "model.h"

/*
 * Implements instanced rendering through the ARB_instanced_arrays path
 */
template<typename... Attribs>
class RenderBatch {
	size_t size;
	Model model;
	//TODO: how to handle copy ctor assign-op?
	GLuint vao_test;
	InterleavedBuffer<Layout::PACKED, Attribs...> attributes;
	std::array<int, sizeof...(Attribs)> indices;

public:
	/*
	 * Struct that stores information about an index to update and the data
	 * to update it with
	 */
	struct Update {
		size_t index;
		std::tuple<Attribs...> attribs;

		Update(size_t index, const std::tuple<Attribs...> &attribs)
			: index(index), attribs(attribs)
		{}
	};

	/*
	 * Create a render batch with some capacity for the passed in model
	 */
	RenderBatch(size_t capacity, Model model) : size(0), model(model),
		attributes(capacity, GL_ARRAY_BUFFER, GL_STREAM_DRAW)
	{
		indices.fill(-1);
	}
	/*
	 * Add instances to be drawn. If the new size exceeds the instance
	 * data buffer capacity it will be re-size (an expensive operation)
	 */
	void push_back(const std::vector<std::tuple<Attribs...>> &objs){
		if (size + objs.size() > attributes.size()){
			resize_buffer(2 * attributes.size());
		}
		attributes.map(GL_WRITE_ONLY);
		for (size_t i = 0; i < objs.size(); ++i){
			attributes.write(size + i, objs[i]);
		}
		attributes.unmap();
		size += objs.size();
	}
	/**
	 * Add a single instance to be drawn. Try not to use this function as
	 * the buffer must be mapped and unmapped to write a single item,
	 * which is slow
	 */
	void push_back(const std::tuple<Attribs...> &obj){
		if (size + 1 > attributes.size()){
			resize_buffer(2 * attributes.size());
		}
		attributes.map(GL_WRITE_ONLY);
		attributes.write(size, obj);
		attributes.unmap();
		++size;
	}
	/*
	 * Update existing instances with new data, specifying the indices to be updated
	 */
	void update(const std::vector<Update> &updates){
		//Should we instead use FLUSH_EXPLICIT here and then condense the flushes into
		//blocks of the updated ranges of attributes?
		attributes.map(GL_WRITE_ONLY);
		for (const Update &u : updates){
			assert(u.index < size);
			attributes.write(u.index, u.attribs);
		}
		attributes.unmap();
	}
	/*
	 * Update existing instances with new data in order
	 */
	void update(const std::vector<std::tuple<Attribs...>> &updates){
		assert(updates.size() <= size);
		attributes.map_range(0, updates.size(), GL_MAP_WRITE_BIT);
		for (size_t i = 0; i < updates.size(); ++i){
			attributes.write(i, updates[i]);
		}
		attributes.unmap();
	}
	/*
	 * Update a single instance with new data
	 */
	void update(size_t i, const std::tuple<Attribs...> &u){
		attributes.map_range(i, 1, GL_MAP_WRITE_BIT);
		attributes.write(i, u);
		attributes.unmap();
	}
	/*
	 * Resize the batch to some new size
	 */
	void resize(size_t n){
		if (n > attributes.size()){
			resize_buffer(n);
		}
		size = n;
	}
	/*
	 * Remove some number of instances at the back of the list, default of 1
	 */
	void pop_back(size_t n = 1){
		size -= n;
	}
	/*
	 * Remove the instance at some index
	 */
	void remove(size_t i){
		assert(i < size);
		attributes.map(GL_WRITE_ONLY);
		for (size_t j = i; j < size - 1; ++j){
			attributes.write(j, attributes.read(j + 1));
		}
		attributes.unmap();
		--size;
	}
	/*
	 * Set the attribute index to send the attributes too
	 */
	void set_attrib_indices(const std::array<int, sizeof...(Attribs)> &i){
		indices = i;
		model.bind();
		attributes.bind();
		set_attrib_index<Attribs...>();
	}
	/*
	 * Render the batch
	 */
	void render(){
		model.bind();
		glDrawElementsInstanced(GL_TRIANGLES, model.elems(), GL_UNSIGNED_SHORT, 0, size);
	}
	size_t batch_size() const {
		return size;
	}

private:
	//Resize the instance data buffer capacity to some new size
	void resize_buffer(size_t n){
		std::cerr << "Performance warning: RenderBatch attribute buffer resizing, consider making it bigger\n";
		InterleavedBuffer<Layout::PACKED, Attribs...> new_buf(n, GL_ARRAY_BUFFER,
			GL_STREAM_DRAW);
		new_buf.bind(GL_COPY_WRITE_BUFFER);
		attributes.bind(GL_COPY_READ_BUFFER);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
			size * detail::Size<Layout::PACKED, Attribs...>::size());
		attributes = new_buf;
		//If the attribute index has been set update the vao with the new buffer
		if (indices[0] > -1){
			set_attrib_indices(indices);
		}
	}
	/*
	 * Recurse through the types in the attribute buffer and set their indices
	 */
	template<typename T>
	void set_attrib_index(){
		int index = sizeof...(Attribs) - 1;
		size_t base_offset = attributes.offset(index);
		GLenum gl_type = detail::gl_attrib_type<T>();
		//number of occupied indices is rounded based on vec4
		//would we want to use the Sizer for this?
		size_t attrib_size = detail::Size<Layout::PACKED, T>::size();
		size_t num_indices = attrib_size / sizeof(glm::vec4);
		num_indices = attrib_size % sizeof(glm::vec4) == 0 ? num_indices : num_indices + 1;
		for (size_t i = 0; i < num_indices; ++i){
			glEnableVertexAttribArray(i + indices[index]);
			if (gl_type == GL_FLOAT || gl_type == GL_HALF_FLOAT || gl_type == GL_DOUBLE){
				//TODO: How should we work through computing the number of values we're sending?
				//or is just saying 4 fine
				glVertexAttribPointer(i + indices[index], 4, gl_type, GL_FALSE, attributes.stride(),
						(void*)(base_offset + sizeof(glm::vec4) * i));
			}
			else {
				glVertexAttribIPointer(i + indices[index], 4, gl_type, attributes.stride(),
						(void*)(base_offset + sizeof(glm::vec4) * i));
			}
			glVertexAttribDivisor(i + indices[index], 1);
		}
	}
	template<typename A, typename B, typename... Args>
	void set_attrib_index(){
		int index = sizeof...(Attribs) - sizeof...(Args) - 2;
		size_t base_offset = attributes.offset(index);
		GLenum gl_type = detail::gl_attrib_type<A>();
		//number of occupied indices is rounded based on vec4
		//would we want to use the Sizer for this?
		size_t attrib_size = detail::Size<Layout::PACKED, A>::size();
		size_t num_indices = attrib_size / sizeof(glm::vec4);
		num_indices = attrib_size % sizeof(glm::vec4) == 0 ? num_indices : num_indices + 1;
		for (size_t i = 0; i < num_indices; ++i){
			glEnableVertexAttribArray(i + indices[index]);
			if (gl_type == GL_FLOAT || gl_type == GL_HALF_FLOAT || gl_type == GL_DOUBLE){
				//TODO: How should we work through computing the number of values we're sending?
				//or is just saying 4 fine
				glVertexAttribPointer(i + indices[index], 4, gl_type, GL_FALSE, attributes.stride(),
						(void*)(base_offset + sizeof(glm::vec4) * i));
			}
			else {
				glVertexAttribIPointer(i + indices[index], 4, gl_type, attributes.stride(),
						(void*)(base_offset + sizeof(glm::vec4) * i));
			}
			glVertexAttribDivisor(i + indices[index], 1);
			//Check that we didn't spill over into another attributes index space
			if (i + indices[index] >= indices[index + 1]){
				std::cerr << "RenderBatch Warning: attribute " << indices[index]
					<< " spilled over into attribute " << indices[index + 1] << std::endl;
			}
		}
		set_attrib_index<B, Args...>();
	}
};

#endif

