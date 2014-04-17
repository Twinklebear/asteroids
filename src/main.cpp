#include <iostream>
#include <tuple>
#include <SDL.h>
#include <entityx/entityx.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "gl_core_3_3.h"
#include "util.h"

struct Position : entityx::Component<Position> {
	float x, y;
	
	Position(float x, float y) : x(x), y(y) {}
};

/**
 * A fixed capacity render batch, each instance only has a distinct model matrix
 * items can be removed but order will not be preserved
 */
class ArrayBatch {
	size_t size, capacity;
	//Should also add a vbo here to hold the model being drawn
	//but since I'm generating the triangles in the shader skip for now
	GLuint vao, matrices, program;

public:
	ArrayBatch(size_t capacity, GLuint program)
		: size(0), capacity(capacity), vao(0), matrices(0), program(program){
		//Defer creating buffers until an object is added
	}
	~ArrayBatch(){
		if (vao != 0){
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &matrices);
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
		glBindBuffer(GL_ARRAY_BUFFER, matrices);
		glm::mat4 *mats = static_cast<glm::mat4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
		for (size_t i = 0; i < objs.size(); ++i){
			mats[i + size] = objs[i];
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
		size += objs.size();
	}
	//Remove the object at some index, this does not preserve render order
	void remove(size_t i){
		if (size == 0){
			std::cerr << "Attempt to delete object on empty batch\n";
			return;
		}
		glBindBuffer(GL_ARRAY_BUFFER, matrices);
		glm::mat4 *mats = static_cast<glm::mat4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
		std::swap(mats[i], mats[size - 1]);
		--size;
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	//Set the attribute index to send matrices too
	void set_attrib_index(unsigned attrib){
		glBindVertexArray(vao);
		for (unsigned i = attrib; i < attrib + 4; ++i){
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
				(void*)(sizeof(glm::vec4) * i));
			glVertexAttribDivisor(i, 1);
		}
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
		glGenBuffers(1, &matrices);
		glBindBuffer(GL_ARRAY_BUFFER, matrices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * capacity, NULL, GL_STREAM_DRAW);
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
		glm::translate<GLfloat>(-0.5f, 0.f, 0.f) * glm::scale<GLfloat>(0.5f, 0.5f, 1.f),
		glm::translate<GLfloat>(0.5f, 0.f, 0.f) * glm::scale<GLfloat>(0.5f, 0.5f, 1.f),
		glm::translate<GLfloat>(0.0f, 0.5f, 0.f) * glm::scale<GLfloat>(0.5f, 0.5f, 1.f),
		glm::translate<GLfloat>(0.0f, -0.5f, 0.f) * glm::scale<GLfloat>(0.5f, 0.5f, 1.f)
	};
	batch.add_objects(matrices);
	batch.set_attrib_index(0);

	bool quit = false;
	while (!quit){
		SDL_Event e;
		while (SDL_PollEvent(&e)){
			if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE){
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

