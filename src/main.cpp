#include <iostream>
#include <tuple>
#include <SDL.h>
#include <entityx/entityx.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "gl_core_3_3.h"
#include "util.h"
#include "interleavedbuffer.h"

/**
 * A fixed capacity render batch, each instance only has a distinct model matrix
 * items can be removed but order will not be preserved
 */
class ArrayBatch {
	size_t size, capacity;
	//Should also add a vbo here to hold the model being drawn
	//but since I'm generating the triangles in the shader skip for now
	GLuint vao, program;
	InterleavedBuffer<glm::mat4, int> matrices;

public:
	ArrayBatch(size_t capacity, GLuint program) : size(0), capacity(capacity),
		vao(0), program(program), matrices(capacity, GL_ARRAY_BUFFER, GL_STREAM_DRAW)
	{}
	~ArrayBatch(){
		if (vao != 0){
			glDeleteVertexArrays(1, &vao);
		}
		glDeleteProgram(program);
	}
	//Add a list of objects to be drawn
	void add_objects(const std::vector<glm::mat4> &objs){
		if (size + objs.size() > capacity){
			std::cerr << "Too many objects to insert into batch\n";
			return;
		}
		if (vao == 0){
			create_buffer();
		}
		matrices.map(GL_WRITE_ONLY);
		for (size_t i = 0; i < objs.size(); ++i){
			matrices.write<0>(i + size) = objs[i];
			matrices.write<1>(i + size) = i % 4;
		}
		matrices.unmap();
		size += objs.size();
	}
	void remove(size_t i){
		if (size == 0){
			std::cerr << "Attempt to delete object on empty batch\n";
			return;
		}
		assert(i < size);
		matrices.map(GL_WRITE_ONLY);
		for (size_t j = i; j < size - 1; ++j){
			matrices.write<0>(j) = matrices.write<0>(j + 1);
			matrices.write<1>(j) = matrices.write<1>(j + 1);
		}
		matrices.unmap();
		--size;
	}
	//Set the attribute index to send matrices too
	void set_attrib_index(unsigned attrib){
		glBindVertexArray(vao);
		matrices.bind();
		for (unsigned i = attrib; i < attrib + 4; ++i){
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, matrices.stride(),
				(void*)(sizeof(glm::vec4) * i));
			glVertexAttribDivisor(i, 1);
		}
		glEnableVertexAttribArray(attrib + 4);
		glVertexAttribIPointer(attrib + 4, 1, GL_INT, matrices.stride(),
			(void*)(detail::Offset<1, glm::mat4, int>::offset()));
		glVertexAttribDivisor(attrib + 4, 1);
	}
	//Render the batch
	void render(){
		glUseProgram(program);
		glBindVertexArray(vao);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 3, size);
	}

private:
	void create_buffer(){
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
};

int main(int argc, char **argv){
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		std::cerr << "SDL_Init error: " << SDL_GetError() << "\n";
		return 1;
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	SDL_Window *win = SDL_CreateWindow("Asteroids", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(win);

	if (ogl_LoadFunctions() == ogl_LOAD_FAILED){
		std::cerr << "ogl load failed\n";
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(win);
		SDL_Quit();
		return 1;
	}
	glClearColor(0, 0, 0, 1);

	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n"
		<< "OpenGL Vendor: " << glGetString(GL_VENDOR) << "\n"
		<< "OpenGL Renderer: " << glGetString(GL_RENDERER) << "\n"
		<< "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageCallbackARB(util::gldebug_callback, NULL);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
		NULL, GL_TRUE);

	GLint program = util::load_program({std::make_tuple(GL_VERTEX_SHADER, "../res/vertex.glsl"),
		std::make_tuple(GL_FRAGMENT_SHADER, "../res/fragment.glsl")});
	if (program == -1){
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(win);
		SDL_Quit();
		return 1;
	}
	ArrayBatch batch(4, program);
	std::vector<glm::mat4> matrices = {
		glm::translate(glm::vec3{-0.5f, 0.f, 0.f})
			* glm::scale(glm::vec3{0.5f, 0.5f, 1.f}),
		glm::translate(glm::vec3{0.5f, 0.f, 0.f})
			* glm::scale(glm::vec3{0.5f, 0.5f, 1.f}),
		glm::translate(glm::vec3{0.0f, 0.5f, 0.f})
			* glm::scale(glm::vec3{0.5f, 0.5f, 1.f}),
		glm::translate(glm::vec3{0.0f, -0.5f, 0.f})
			* glm::scale(glm::vec3{0.5f, 0.5f, 1.f})
	};
	batch.add_objects(matrices);
	batch.set_attrib_index(0);

	bool quit = false;
	while (!quit){
		SDL_Event e;
		while (SDL_PollEvent(&e)){
			if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)){
				quit = true;
				break;
			}
			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_w){
				batch.remove(0);
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		batch.render();

		GLenum err = glGetError();
		if (err != GL_NO_ERROR){
			std::cerr << "OpenGL Error: " << std::hex << err << std::dec << "\n";
		}
		SDL_GL_SwapWindow(win);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}

