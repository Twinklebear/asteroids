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
	size_t size, capacity;
	Model model;
	GLuint vao_test;
	InterleavedBuffer<Layout::ALIGNED, glm::mat4> matrices;

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
	 * Update existing instances with new data
	 */
	void update(const std::vector<std::tuple<size_t, glm::mat4>> &updates);
	/*
	 * Update a single instance with new data
	 */
	void update(size_t i, const glm::mat4 &mat);
	/*
	 * Remove instance at the back of the list
	 */
	void pop_back();
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
	//Double the instance data buffer capacity
	void resize_buffer();
};

#endif

