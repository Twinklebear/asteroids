#include <vector>
#include <array>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <tuple>
#include <glm/glm.hpp>
#include <SDL.h>
#include "gl_core_3_3.h"
#include "util.h"

std::string util::read_file(const std::string &fName){
	std::ifstream file(fName);
	if (!file.is_open()){
		std::cout << "Failed to open file: " << fName << std::endl;
		return "";
	}
	return std::string((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());
}
GLint util::load_shader(GLenum type, const std::string &file){
	GLuint shader = glCreateShader(type);
	std::string src = read_file(file);
	const char *csrc = src.c_str();
	glShaderSource(shader, 1, &csrc, 0);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE){
		std::cerr << "loadShader: ";
		switch (type){
		case GL_VERTEX_SHADER:
			std::cerr << "Vertex shader: ";
			break;
		case GL_FRAGMENT_SHADER:
			std::cerr << "Fragment shader: ";
			break;
		case GL_GEOMETRY_SHADER:
			std::cerr << "Geometry shader: ";
			break;
		default:
			std::cerr << "Other shader type: ";
		}
		std::cerr << file << " failed to compile. Compilation log:\n";
		GLint len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		char *log = new char[len];
		glGetShaderInfoLog(shader, len, 0, log);
		std::cerr << log << "\n";
		delete[] log;
		glDeleteShader(shader);
		return -1;
	}
	return shader;
}
GLint util::load_program(const std::vector<std::tuple<GLenum, std::string>> &shaders){
	std::vector<GLuint> glshaders;
	for (const std::tuple<GLenum, std::string> &s : shaders){
		GLint h = load_shader(std::get<0>(s), std::get<1>(s));
		if (h == -1){
			std::cerr << "loadProgram: A required shader failed to compile, aborting\n";
			for (GLuint g : glshaders){
				glDeleteShader(g);
			}
			return -1;
		}
		glshaders.push_back(h);
	}
	GLuint program = glCreateProgram();
	for (GLuint s : glshaders){
		glAttachShader(program, s);
	}
	glLinkProgram(program);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE){
		std::cerr << "loadProgram: Program failed to link, log:\n";
		GLint len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		char *log = new char[len];
		glGetProgramInfoLog(program, len, 0, log);
		std::cerr << log << "\n";
		delete[] log;
	}
	for (GLuint s : glshaders){
		glDetachShader(program, s);
		glDeleteShader(s);
	}
	if (status == GL_FALSE){
		glDeleteProgram(program);
		return -1;
	}
	return program;
}
GLuint util::load_texture(const std::string &file){
	SDL_Surface *surf = SDL_LoadBMP(file.c_str());
	//TODO: Throw an error?
	if (!surf){
		std::cout << "Failed to load bmp: " << file
			<< " SDL_error: " << SDL_GetError() << "\n";
		return 0;
	}
	//Assume 4 or 3 bytes per pixel
	GLenum format, internal;
	if (surf->format->BytesPerPixel == 4){
		internal = GL_RGBA;
		if (surf->format->Rmask == 0x000000ff){
			format = GL_RGBA;
		}
		else {
			format = GL_BGRA;
		}
	}
	else {
		internal = GL_RGB;
		if (surf->format->Rmask == 0x000000ff){
			format = GL_RGB;
		}
		else {
			format = GL_BGR;
		}
	}
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, internal, surf->w, surf->h, 0, format,
		GL_UNSIGNED_BYTE, surf->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surf);
	return tex;
}
bool util::log_glerror(const std::string &msg){
	GLenum err = glGetError();
	if (err != GL_NO_ERROR){
		std::cerr << "OpenGL Error: ";
		switch (err){
		case GL_INVALID_ENUM:
			std::cerr << "Invalid enum";
			break;
		case GL_INVALID_VALUE:
			std::cerr << "Invalid value";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << "Invalid operation";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << "Out of memory";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cerr << "Invalid FrameBuffer operation";
			break;
		default:
			std::cerr << std::hex << err << std::dec;
		}
		std::cerr << " - " << msg << "\n";
		return true;
	}
	return false;
}
#if _MSC_VER
void APIENTRY util::gldebug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
	GLsizei len, const GLchar *msg, const GLvoid *user)
#else
void util::gldebug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
	GLsizei len, const GLchar *msg, const GLvoid *user)
#endif
{
	//Print a time stamp for the message
	float sec = SDL_GetTicks() / 1000.f;
	int min = static_cast<int>(sec / 60.f);
	sec -= sec / 60.f;
	std::cerr << "[" << min << ":"
		<< std::setprecision(3) << sec << "] OpenGL Debug -";
	switch (severity){
	case GL_DEBUG_SEVERITY_HIGH_ARB:
		std::cerr << " High severity";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM_ARB:
		std::cerr << " Medium severity";
		break;
	case GL_DEBUG_SEVERITY_LOW_ARB:
		std::cerr << " Low severity";
	}
	switch (src){
	case GL_DEBUG_SOURCE_API_ARB:
		std::cerr << " API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
		std::cerr << " Window system";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
		std::cerr << " Shader compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
		std::cerr << " Third party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION_ARB:
		std::cerr << " Application";
		break;
	default:
		std::cerr << " Other";
	}
	switch (type){
	case GL_DEBUG_TYPE_ERROR_ARB:
		std::cerr << " Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
		std::cerr << " Deprecated behavior";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
		std::cerr << " Undefined behavior";
		break;
	case GL_DEBUG_TYPE_PORTABILITY_ARB:
		std::cerr << " Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE_ARB:
		std::cerr << " Performance";
		break;
	default:
		std::cerr << " Other";
	}
	std::cerr << ":\n\t" << msg << "\n";
}
bool util::load_obj(const std::string &fname,
	InterleavedBuffer<Layout::PACKED, glm::vec3, glm::vec3, glm::vec3> &vbo,
	InterleavedBuffer<Layout::PACKED, GLushort> &ebo, size_t &n_elems)
{
	std::ifstream file(fname);
	if (!file.is_open()){
		std::cout << "Failed to find obj file: " << fname << std::endl;
		return false;
	}
	//Temporary storage for the data we read in
	std::vector<glm::vec3> tmp_pos, tmp_norm;
	std::vector<glm::vec2> tmp_uv;
	//A map to associate a unique vertex with its index
	std::map<std::string, GLushort> vert_indices;
	//The final ordered packed vertices and indices
	std::vector<glm::vec3> vert_data;
	std::vector<GLushort> indices;

	std::string line;
	while (std::getline(file, line)){
		if (line.empty()){
			continue;
		}
		//Parse vertex info: positions, uv coords and normals
		else if (line.at(0) == 'v'){
			//positions
			if (line.at(1) == ' '){
				tmp_pos.push_back(capture_vec3(line));
			}
			else if (line.at(1) == 't'){
				tmp_uv.push_back(capture_vec2(line));
			}
			else if (line.at(1) == 'n'){
				tmp_norm.push_back(capture_vec3(line));
			}
		}
		//Parse faces
		else if (line.at(0) == 'f'){
			std::array<std::string, 3> face = capture_faces(line);
			for (std::string &v : face){
				auto fnd = vert_indices.find(v);
				//If we find the vertex already in the list re-use the index
				//If not we create a new vertex and index
				if (fnd != vert_indices.end()){
					indices.push_back(fnd->second);
				}
				else {
					std::array<unsigned int, 3> vertex = capture_vertex(v);
					//Pack the position, normal and uv into the vertex data, note that obj data is
					//1-indexed so we subtract 1
					vert_data.push_back(tmp_pos[vertex[0] - 1]);
					vert_data.push_back(tmp_norm[vertex[2] - 1]);
					vert_data.push_back(glm::vec3(tmp_uv[vertex[1] - 1], 0));
					//Store the new index, also subract 1 b/c size 1 => idx 0
					//and divide by 3 b/c there are 3 components per vertex
					indices.push_back((vert_data.size() - 1) / 3);
					vert_indices[v] = indices.back();
				}
			}
		}
	}
	n_elems = indices.size();
	vbo = InterleavedBuffer<Layout::PACKED, glm::vec3, glm::vec3, glm::vec3>(n_elems, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	vbo.map(GL_WRITE_ONLY);
	for (size_t i = 0; i < n_elems; ++i){
		vbo.write<0>(i) = vert_data[3 * i];
		vbo.write<1>(i) = vert_data[3 * i + 1];
		vbo.write<2>(i) = vert_data[3 * i + 2];
	}
	vbo.unmap();
	ebo = InterleavedBuffer<Layout::PACKED, GLushort>(n_elems, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
	ebo.map(GL_WRITE_ONLY);
	for (size_t i = 0; i < indices.size(); ++i){
		ebo.write<0>(i) = indices[i];
	}
	ebo.unmap();
	return true;
}
glm::vec2 util::capture_vec2(const std::string &str){
	glm::vec2 vec;
	sscanf(str.c_str(), "%*s %f %f", &vec.x, &vec.y);
	return vec;
}
glm::vec3 util::capture_vec3(const std::string &str){
	glm::vec3 vec;
	sscanf(str.c_str(), "%*s %f %f %f", &vec.x, &vec.y, &vec.z);
	return vec;
}
std::array<std::string, 3> util::capture_faces(const std::string &str){
	std::array<std::string, 3> faces;
	//There's face information between each space in the string, and 3 faces total
	size_t prev = str.find(" ", 0);
	size_t next = prev;
	for (std::string &face : faces){
		next = str.find(" ", prev + 1);
		face = str.substr(prev + 1, next - prev - 1);
		prev = next;
	}
	return faces;
}
std::array<unsigned int, 3> util::capture_vertex(const std::string &str){
	std::array<unsigned int, 3> vertex;
	sscanf(str.c_str(), "%u/%u/%u", &vertex[0], &vertex[1], &vertex[2]);
	return vertex;
}
