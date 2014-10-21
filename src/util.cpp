#include <vector>
#include <regex>
#include <array>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <tuple>
#include <glm/glm.hpp>
#include <SDL.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "gl_core_3_3.h"
#include "util.h"

std::string util::get_resource_path(const std::string &sub_dir){
	static std::string base_res;
	if (base_res.empty()){
		char *base_path = SDL_GetBasePath();
		if (base_path){
			base_res = base_path;
			SDL_free(base_path);
		}
		else {
			std::cerr << "Error getting resource path: " << SDL_GetError() << std::endl;
			return "";
		}
		//The final part of the string should be bin/ so replace it with res/
		size_t pos = base_res.rfind("bin");
		base_res = base_res.substr(0, pos) + "res" + PATH_SEP;
	}
	if (sub_dir.empty()){
		return base_res;
	}
	return base_res + sub_dir + PATH_SEP;
}
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
		std::cerr << "load_shader: ";
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
			std::cerr << "load_program: A required shader failed to compile, aborting\n";
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
		std::cerr << "load_program: Program failed to link, log:\n";
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
//Swap rows of n bytes pointed to by a with those pointed to by b
//for use in doing the y-flip for images so OpenGL has them right-side up
void swap_row(unsigned char *a, unsigned char *b, size_t n){
	for (size_t i = 0; i < n; ++i){
		std::swap(a[i], b[i]);
	}
}
GLuint util::load_texture(const std::string &file, size_t *width, size_t *height){
	int x, y, n;
	unsigned char *img = stbi_load(file.c_str(), &x, &y, &n, 0);
	if (!img){
		std::cerr << "Failed to load image " << file
			<< stbi_failure_reason() << std::endl;
		return 0;
	}
	if (width){
		*width = x;
	}
	if (height){
		*height = y;
	}
	GLenum format;
	switch (n){
		case 1:
			format = GL_RED;
			break;
		case 2:
			format = GL_RG;
			break;
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
	}
	for (int i = 0; i < y / 2; ++i){
		swap_row(&img[i * x * n], &img[(y - i - 1) * x * n], x * n);
	}

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, format, x, y, 0, format, GL_UNSIGNED_BYTE, img);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(img);
	return tex;
}
GLuint util::load_texture_array(const std::vector<std::string> &files, size_t *w, size_t *h){
	assert(!files.empty());
	int x, y, n;
	std::vector<unsigned char*> images;
	//We need to load the first image to get the dimensions and format we're loading
	images.push_back(stbi_load(files.front().c_str(), &x, &y, &n, 0));
	if (w){
		*w = x;
	}
	if (h){
		*h = y;
	}
	for (auto it = ++files.begin(); it != files.end(); ++it){
		int ix, iy, in;
		images.push_back(stbi_load(it->c_str(), &ix, &iy, &in, 0));
		if (x != ix || y != iy || n != in){
			std::cerr << "load_texture_array error: Attempt to create array of incompatible images\n";
			for (auto i : images){
				stbi_image_free(i);
			}
			return 0;
		}
	}
	//Perform y-swap on each loaded image
	for (auto img : images){
		for (int i = 0; i < y / 2; ++i){
			swap_row(&img[i * x * n], &img[(y - i - 1) * x * n], x * n);
		}
	}
	GLenum format;
	switch (n){
		case 1:
			format = GL_RED;
			break;
		case 2:
			format = GL_RG;
			break;
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
	}

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, x, y, images.size(), 0, format, GL_UNSIGNED_BYTE, NULL);
	//Upload all the textures in the array
	for (size_t i = 0; i < images.size(); ++i){
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, x, y, 1, format, GL_UNSIGNED_BYTE, images.at(i));
	}
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	//Clean up all the image data
	for (auto i : images){
		stbi_image_free(i);
	}
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
			std::vector<std::string> face = capture_faces(line);
			//Triangulate quad faces
			if (face.size() == 4){
				face.push_back(face.at(0));
				face.push_back(face.at(2));
			}
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
	vbo.reserve(vert_data.size() / 3);
	vbo.map(GL_WRITE_ONLY);
	for (size_t i = 0; i < vert_data.size() / 3; ++i){
		vbo.write<0>(i) = vert_data[3 * i];
		vbo.write<1>(i) = vert_data[3 * i + 1];
		vbo.write<2>(i) = vert_data[3 * i + 2];
	}
	vbo.unmap();
	n_elems = indices.size();
	ebo.reserve(n_elems);
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
std::vector<std::string> util::capture_faces(const std::string &str){
	std::regex match_vert("([0-9]+)/([0-9]+)/([0-9]+)");
	std::vector<std::string> faces;
	std::transform(std::sregex_iterator{str.begin(), str.end(), match_vert},
		std::sregex_iterator{}, std::back_inserter(faces),
		[](const std::smatch &m){
			return m.str();
		});
	return faces;
}
std::array<unsigned int, 3> util::capture_vertex(const std::string &str){
	std::array<unsigned int, 3> vertex;
	sscanf(str.c_str(), "%u/%u/%u", &vertex[0], &vertex[1], &vertex[2]);
	return vertex;
}
