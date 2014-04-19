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
#include "renderbatch.h"

void run(SDL_Window *win);

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

	run(win);

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
void run(SDL_Window *win){
	GLint program = util::load_program({std::make_tuple(GL_VERTEX_SHADER, "../res/vertex.glsl"),
		std::make_tuple(GL_FRAGMENT_SHADER, "../res/fragment.glsl")});
	if (program == -1){
		return;
	}
	glUseProgram(program);

	RenderBatch batch(4);
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
	batch.push_back(matrices);
	batch.set_attrib_index(0);

	bool quit = false;
	while (!quit){
		SDL_Event e;
		while (SDL_PollEvent(&e)){
			if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)){
				quit = true;
				break;
			}
			if (e.type == SDL_KEYDOWN && batch.batch_size() > 0){
				if (e.key.keysym.sym == SDLK_w){
					batch.pop_back();
					matrices.pop_back();
				}
				if (e.key.keysym.sym == SDLK_a){
					std::vector<std::tuple<size_t, glm::mat4>> updates;
					for (size_t i = 0; i < batch.batch_size(); ++i){
						matrices[i] = glm::translate(glm::vec3{-0.01f, 0.f, 0.f}) * matrices[i];
						updates.push_back(std::make_tuple(i, matrices[i]));
					}
					batch.update(updates);
				}
				if (e.key.keysym.sym == SDLK_d){
					std::vector<std::tuple<size_t, glm::mat4>> updates;
					for (size_t i = 0; i < batch.batch_size(); ++i){
						matrices[i] = glm::translate(glm::vec3{0.01f, 0.f, 0.f}) * matrices[i];
						updates.push_back(std::make_tuple(i, matrices[i]));
					}
					batch.update(updates);
				}
				if (e.key.keysym.sym == SDLK_e){
					matrices.push_back(glm::translate(glm::vec3{0.f, 0.f, 0.f})
						* glm::scale(glm::vec3{0.5f, 0.5f, 1.f}));
					batch.push_back(matrices.back());
					batch.set_attrib_index(0);
				}
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
	glDeleteProgram(program);
}

