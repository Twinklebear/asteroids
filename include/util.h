#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <array>
#include <string>
#include <glm/glm.hpp>
#include "gl_core_3_3.h"
#include "interleavedbuffer.h"

namespace util {
#ifdef _WIN32
	const char PATH_SEP = '\\';
#else
	const char PATH_SEP = '/';
#endif
	//Convert degrees to radians
	constexpr float deg_to_rad(float deg){
		return deg * 0.01745f;
	}
	/*
	 * Get the resource path for resources located in res/sub_dir
	 */
	std::string get_resource_path(const std::string &sub_dir = "");
	/*
	* Read the entire contents of a file into a string, if an error occurs
	* the string will be empty
	*/
	std::string read_file(const std::string &fName);
	/*
	 * Load a GLSL shader from some file, returns -1 if loading failed
	 */
	GLint load_shader(GLenum type, const std::string &file);
	/*
	 * Build a shader program from the list of shaders passed
	 */
	GLint load_program(const std::vector<std::tuple<GLenum, std::string>> &shaders);
	/*
	 * Load an image into a 2D texture, creating a new texture id
	 * The texture unit desired for this texture should be set active
	 * before loading the texture as it will be bound during the loading process
	 * Can also optionally pass width & height variables to return the width
	 * and height of the loaded image
	 */
	GLuint load_texture(const std::string &file, size_t *width = nullptr, size_t *height = nullptr);
	/*
	 * Load a series of images into a 2D texture array, creating a new texture id
	 * The images will appear in the array in the same order they're passed in
	 * It is an error if the images don't all have the same dimensions
	 * or have different formats
	 * The texture unit desired for this texture should be set active
	 * before loading the texture as it will be bound during the loading process
	 * Can also optionally pass width & height variables to return the width
	 * and height of the loaded image
	 */
	GLuint load_texture_array(const std::vector<std::string> &files, size_t *w = nullptr, size_t *h = nullptr);
	/*
	 * Check for an OpenGL error and log it along with the message passed
	 * if an error occured. Will return true if an error occured & was logged
	 */
	bool log_glerror(const std::string &msg);
	/*
	 * A debug callback for the GL_ARB_debug_out extension
	 */
#ifdef _WIN32
	void APIENTRY gldebug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
		GLsizei len, const GLchar *msg, const GLvoid *user);
#else
	void gldebug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
		GLsizei len, const GLchar *msg, const GLvoid *user);
#endif
	/*
	* Load an OBJ model file into the vbo and ebo passed in
	* The model must have vertex, texture and normal data and be a triangle mesh
	* The vbo elems are: vec3 pos, vec3 normal, vec3 uv
	* returns true on success, false on failure
	* TODO: Take any buffer layout?
	*/
	bool load_obj(const std::string &fname,
		InterleavedBuffer<Layout::PACKED, glm::vec3, glm::vec3, glm::vec3> &vbo,
		InterleavedBuffer<Layout::PACKED, GLushort> &ebo, size_t &n_elems);
	/*
	* Functions to get values from formatted strings, for use in reading the
	* model file
	*/
	glm::vec2 capture_vec2(const std::string &str);
	glm::vec3 capture_vec3(const std::string &str);
	/*
	* Get the vertex informaton for a face as an array of 3 strings
	*/
	std::array<std::string, 3> capture_faces(const std::string &str);
	/*
	* Capture the indices of the vertex components, the array contains the
	* face information: v/vt/vn
	*/
	std::array<unsigned int, 3> capture_vertex(const std::string &str);
}

#endif
