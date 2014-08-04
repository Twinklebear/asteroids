#include <iostream>
#include <array>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "util.h"
#include "layout_offset.h"
#include "interleavedbuffer.h"
#include "model.h"

Model::Model(const std::string &file) : vao(0), vbo(0, GL_ARRAY_BUFFER, GL_STATIC_DRAW),
	ebo(0, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW), n_elems(0)
{
	glGenVertexArrays(1, &vao);
	load(file);
}
Model::~Model(){
	glDeleteVertexArrays(1, &vao);
}
Model::Model(Model &&m): vao(m.vao), vbo(std::move(m.vbo)),
	ebo(std::move(m.ebo)), n_elems(m.n_elems)
{
	m.dump_model();
}
Model& Model::operator=(Model &&m){
	if (this != &m){
		vao = m.vao;
		vbo = std::move(m.vbo);
		ebo = std::move(m.ebo);
		n_elems = m.n_elems;
		m.dump_model();
	}
	return *this;
}
void Model::bind(){
	glBindVertexArray(vao);
}
size_t Model::elems(){
	return n_elems;
}
void Model::load(const std::string &file){
	glBindVertexArray(vao);
	if (!util::load_obj(file, vbo, ebo, n_elems)){
		std::cerr << "Model " << file << " failed to load\n";
		return;
	}
	vbo.bind();
	ebo.bind();
	for (int i = 0; i < 2; ++i){
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, vbo.stride(), (void*)vbo.offset(i));
	}
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vbo.stride(), (void*)(vbo.offset(2)));
}
void Model::dump_model(){
	vao = 0;
	n_elems = 0;
}

