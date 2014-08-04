#ifndef MODEL_H
#define MODEL_H

#include <array>
#include <memory>
#include <string>
#include "interleavedbuffer.h"

/*
 * A lightweight model class, stores the vao, vbo and ebo
 */
class Model {
	GLuint vao;
	InterleavedBuffer<Layout::PACKED, glm::vec3, glm::vec3, glm::vec3> vbo;
	InterleavedBuffer<Layout::PACKED, GLushort> ebo;
	size_t n_elems;

public:
	/*
	 * Load the model from an obj file
	 */
	Model(const std::string &file);
	~Model();
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	/*
	 * Move the vao and buffers from one model object into this one
	 * the old model object will no longer be valid
	 */
	Model(Model &&m);
	Model& operator=(Model &&m);
	void bind();
	size_t elems();

private:
	/*
	 * Load from the obj file and setup the vao
	 */
	void load(const std::string &file);
	/*
	 * Dump ownership of some model data
	 */
	void dump_model();
};

#endif

