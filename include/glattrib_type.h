#ifndef GLATTRIB_TYPE
#define GLATTRIB_TYPE

#include "gl_core_3_3.h"

namespace detail {
template<typename T>
GLenum gl_attrib_type();
template<>
GLenum gl_attrib_type<float>(){
	return GL_FLOAT;
}
template<>
GLenum gl_attrib_type<int>(){
	return GL_INT;
}
template<>
GLenum gl_attrib_type<unsigned>(){
	return GL_UNSIGNED_INT;
}
template<>
GLenum gl_attrib_type<char>(){
	return GL_UNSIGNED_BYTE;
}
template<>
GLenum gl_attrib_type<glm::vec2>(){
	return gl_attrib_type<glm::vec2::value_type>();
}
template<>
GLenum gl_attrib_type<glm::vec3>(){
	return gl_attrib_type<glm::vec3::value_type>();
}
template<>
GLenum gl_attrib_type<glm::vec4>(){
	return gl_attrib_type<glm::vec4::value_type>();
}
template<>
GLenum gl_attrib_type<glm::mat4>(){
	//2x2 and 3x3 matrices get the same column-padding applied as in STD140
	//it's not worth the hassle to support them as attributes
	return gl_attrib_type<glm::mat4::value_type>();
}
}

#endif

