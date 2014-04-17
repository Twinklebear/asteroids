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

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * 2, NULL, GL_STATIC_DRAW);
	glm::mat4 *mats = static_cast<glm::mat4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	mats[0] = glm::translate<GLfloat>(-0.5f, 0.f, 0.f) * glm::scale<GLfloat>(0.5f, 0.5f, 1.f);
	mats[1] = glm::translate<GLfloat>(0.5f, 0.f, 0.f) * glm::scale<GLfloat>(0.5f, 0.5f, 1.f);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	//Enable all columns of the matrices
	for (int i = 0; i < 4; ++i){
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
			(void*)(sizeof(glm::vec4) * i));
		glVertexAttribDivisor(i, 1);
	}

	GLint program = util::load_program({std::make_tuple(GL_VERTEX_SHADER, "../res/vertex.glsl"),
		std::make_tuple(GL_FRAGMENT_SHADER, "../res/fragment.glsl")});
	if (program == -1){
		glDeleteVertexArrays(1, &vao);
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(win);
		SDL_Quit();
		return 1;
	}
	glUseProgram(program);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 2);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR){
		std::cerr << "OpenGL Error: " << std::hex << err << std::dec << "\n";
	}
	SDL_GL_SwapWindow(win);
	SDL_Delay(1000);

	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}

