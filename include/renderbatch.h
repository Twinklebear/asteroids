#ifndef RENDER_BATCH_H
#define RENDER_BATCH_H

#include <glm/glm.hpp>
#include "gl_core_3_3.h"
#include "interleavedbuffer.h"
#include "model.h"

/*
 * Implements instanced rendering of a model where each instance has its
 * own model matrix
 */
class RenderBatch {
	size_t size;
	Model model;
	GLuint vao_test;
	InterleavedBuffer<Layout::PACKED, glm::mat4> matrices;
	int attrib_idx;

public:
	/*
	 * Create a render batch with some capacity for the passed in model
	 */
	RenderBatch(size_t capacity, Model model);
	/*
	 * Add instances to be drawn. If the new size exceeds the instance
	 * data buffer capacity it will be re-size (an expensive operation)
	 */
	void push_back(const std::vector<glm::mat4> &objs);
	/**
	 * Add a single instance to be drawn. Try not to use this function as
	 * the buffer must be mapped and unmapped to write a single item,
	 * which is slow
	 */
	void push_back(const glm::mat4 &mat);
	/*
	 * Update existing instances with new data, specifying the indices to be updated
	 */
	void update(const std::vector<std::tuple<size_t, glm::mat4>> &updates);
	/*
	 * Update existing instances with new data in order
	 */
	void update(const std::vector<glm::mat4> &updates);
	/*
	 * Update a single instance with new data
	 */
	void update(size_t i, const glm::mat4 &mat);
	/*
	 * Resize the batch to some new size
	 */
	void resize(size_t n);
	/*
	 * Remove some number of instances at the back of the list, default of 1
	 */
	void pop_back(size_t n = 1);
	/*
	 * Remove the instance at some index
	 */
	void remove(size_t i);
	/*
	 * Set the attribute index to send the matrices too
	 */
	void set_attrib_index(unsigned attrib);
	/*
	 * Render the batch
	 */
	void render();
	size_t batch_size() const;

private:
	//Resize the instance data buffer capacity to some new size
	void resize_buffer(size_t n);
};

#endif

