#ifndef RENDER_BATCH_H
#define RENDER_BATCH_H

#include <glm/glm.hpp>
#include "gl_core_3_3.h"
#include "interleavedbuffer.h"

/*
 * Implements instanced rendering where each instance has its
 * own model matrix. TODO: Can be optionally given a configured
 * vao and model to render
 */
class RenderBatch {
	size_t size, capacity;
	GLuint vao;
	//TODO: Arbitrary instance data support?
	InterleavedBuffer<glm::mat4> matrices;

public:
	RenderBatch(size_t capacity);
	~RenderBatch();
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
	 * TODO: We need more information about the model structure to
	 * know if we should bind an element buffer, how many triangles, etc
	 */
	void render();
	size_t batch_size() const;

private:
	//Double the instance data buffer capacity
	void resize_buffer();
};

#endif

