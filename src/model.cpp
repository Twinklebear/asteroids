#include <iostream>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "util.h"
#include "buffer_offset.h"
#include "interleavedbuffer.h"
#include "model.h"

Model::Model(const std::string &file) : vao(nullptr), vbo(), ebo(), n_elems(0){
	load(file);
}
void Model::bind(){
	glBindVertexArray(*vao);
}
size_t Model::elems(){
	return n_elems;
}
void Model::load(const std::string &file){
	if (!util::load_obj(file, vbo, ebo, n_elems)){
		std::cerr << "Model " << file << " failed to load\n";
		return;
	}
	vao = std::shared_ptr<GLuint>(new GLuint{0}, detail::delete_vao);
	glGenVertexArrays(1, &(*vao));
	glBindVertexArray(*vao);
	ebo.bind();
	vbo.bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vbo.stride(), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vbo.stride(),
		(void*)(detail::Offset<1, Layout::ALIGNED, glm::vec3, glm::vec3, glm::vec3>::offset()));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vbo.stride(),
		(void*)(detail::Offset<2, Layout::ALIGNED, glm::vec3, glm::vec3, glm::vec3>::offset()));
}

