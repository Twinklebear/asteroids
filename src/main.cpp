#include <iostream>
#include <tuple>
#include <array>
#include <string>
#include <SDL.h>
#include <entityx/entityx.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "gl_core_3_3.h"
#include "util.h"
#include "interleavedbuffer.h"
#include "renderbatch.h"
#include "model.h"
#include "level.h"
#include "layout_padding.h"

void run(SDL_Window *win);
//This is just for testing that the alignments/offsets I compute match STD140 in GLSL
std::string gltype_tostring(GLint type);
void print_glsl_blocks();
void test_buffer();

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
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClearDepth(1.f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n"
		<< "OpenGL Vendor: " << glGetString(GL_VENDOR) << "\n"
		<< "OpenGL Renderer: " << glGetString(GL_RENDERER) << "\n"
		<< "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageCallbackARB(util::gldebug_callback, NULL);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
		NULL, GL_TRUE);

	test_buffer();
	//run(win);

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
void run(SDL_Window *win){
	Level level;
	level.start();
	while (!level.should_quit()){
		//TODO Fake time for now
		level.step(0.1);
		GLenum err = glGetError();
		if (err != GL_NO_ERROR){
			std::cerr << "OpenGL Error: " << std::hex << err << std::dec << "\n";
		}
		SDL_GL_SwapWindow(win);
		//TODO VSync? proper time delays etc?
		SDL_Delay(16);
	}
}

void test_buffer(){
	InterleavedBuffer<Layout::STD140, float, STD140Array<float, 10>> buf{1, GL_ARRAY_BUFFER, GL_STATIC_DRAW};
	
	buf.map(GL_READ_WRITE);
	auto w_block = buf.at(0);
	*std::get<0>(w_block) = 0.5f;
	STD140Array<float, 10> &arr = *std::get<1>(w_block);
	for (size_t i = 0; i < arr.size(); ++i){
		arr[i] = i;
	}
	buf.unmap();
	buf.map(GL_READ_ONLY);
	std::cout << "buf.read<0>(0) = " << buf.read<0>(0) << std::endl;
	const STD140Array<float, 10> &carr = buf.read<1>(0);
	for (size_t i = 0; i < arr.size(); ++i){
		std::cout << "carr[" << i << "] = " << carr[i] << std::endl;
	}
}

using Offset = detail::Offset<Layout::STD140, glm::mat4, float,
	STD140Array<float, 10>, int, STD140Array<int, 5>>;
using Size = detail::Size<Layout::STD140, glm::mat4, float,
	STD140Array<float, 10>, int, STD140Array<int, 5>>;

void print_glsl_blocks(){
	static std::string divider(20, '-');
	std::cout << divider << "\n";
	GLint program = util::load_program({std::make_tuple(GL_VERTEX_SHADER, "../res/vtest.glsl"),
		std::make_tuple(GL_FRAGMENT_SHADER, "../res/ftest.glsl")});
	assert(program != -1);
	GLuint block_idx = glGetUniformBlockIndex(program, "Test");
	assert(block_idx != GL_INVALID_INDEX);
	GLint param;
	glGetActiveUniformBlockiv(program, block_idx, GL_UNIFORM_BLOCK_DATA_SIZE, &param);
	std::cout << "Uniform block Test requires " << param << " bytes\n";

	glGetActiveUniformBlockiv(program, block_idx,
		GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &param);
	std::vector<GLint> indices(param);
	glGetActiveUniformBlockiv(program, block_idx,
		GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, &indices[0]);

	std::cout << "Number of active uniforms: " << indices.size() << "\n";
	for (GLuint i : indices){
		glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_NAME_LENGTH, &param);
		char *name = new char[param];
		glGetActiveUniformName(program, i, param, NULL, name);
		std::cout << divider << "\nUniform index " << i
			<< "\nName: " << name << "\n";
		delete[] name;

		glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_TYPE, &param);
		std::cout << "Type: " << gltype_tostring(param) << "\n";
		glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_SIZE, &param);
		if (param > 1){
			std::cout << "Array length: " << param << "\n";
			glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_ARRAY_STRIDE, &param);
			std::cout << "Array stride: " << param << "\n";
		}
		glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_OFFSET, &param);
		std::cout << "Offset: " << param << "\n";
		glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_MATRIX_STRIDE, &param);
		if (param > 0){
			std::cout << "Matrix stride: " << param << "\n";
		}
	}
	std::cout << divider << "\n";
	glDeleteProgram(program);

	std::cout << "Computed Size: " << Size::size() << "\n";
	std::array<size_t, 5> offsets = Offset::offsets();
	for (size_t i = 0; i < 5; ++i){
		std::cout << "Offset of " << i << " = "
			<< offsets[i] << "\n";
	}
}
std::string gltype_tostring(GLint type){
	switch (type){
		case GL_FLOAT:
			return "GL_FLOAT";
		case GL_FLOAT_VEC2:
			return "GL_FLOAT_VEC2";
		case GL_FLOAT_VEC3:
			return "GL_FLOAT_VEC3";
		case GL_FLOAT_VEC4:
			return "GL_FLOAT_VEC4";
		case GL_UNSIGNED_BYTE:
			return "GL_UNSIGNED_BYTE";
		case GL_INT:
			return "GL_INT";
		case GL_INT_VEC2:
			return "GL_INT_VEC2";
		case GL_INT_VEC3:
			return "GL_INT_VEC3";
		case GL_INT_VEC4:
			return "GL_INT_VEC4";
		case GL_UNSIGNED_INT:
			return "GL_UNSIGNED_INT";
		case GL_UNSIGNED_INT_VEC2:
			return "GL_UNSIGNED_INT_VEC2";
		case GL_UNSIGNED_INT_VEC3:
			return "GL_UNSIGNED_INT_VEC3";
		case GL_UNSIGNED_INT_VEC4:
			return "GL_UNSIGNED_INT_VEC4";
		case GL_BOOL:
			return "GL_BOOL";
		case GL_BOOL_VEC2:
			return "GL_BOOL_VEC2";
		case GL_BOOL_VEC3:
			return "GL_BOOL_VEC3";
		case GL_BOOL_VEC4:
			return "GL_BOOL_VEC4";
		case GL_FLOAT_MAT2:
			return "GL_FLOAT_MAT2";
		case GL_FLOAT_MAT3:
			return "GL_FLOAT_MAT3";
		case GL_FLOAT_MAT4:
			return "GL_FLOAT_MAT4";
		case GL_FLOAT_MAT2x3:
			return "GL_FLOAT_MAT2x3";
		case GL_FLOAT_MAT2x4:
			return "GL_FLOAT_MAT2x4";
		case GL_FLOAT_MAT3x2:
			return "GL_FLOAT_MAT3x2";
		case GL_FLOAT_MAT3x4:
			return "GL_FLOAT_MAT3x4";
		case GL_FLOAT_MAT4x2:
			return "GL_FLOAT_MAT4x2";
		case GL_FLOAT_MAT4x3:
			return "GL_FLOAT_MAT4x3";
		default:
			return "Other Type";
	}
}

