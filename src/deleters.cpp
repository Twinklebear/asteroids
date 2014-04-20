#include "deleters.h"
#include "gl_core_3_3.h"

void detail::delete_buffer(GLuint *b){
	glDeleteBuffers(1, b);
}
void detail::delete_vao(GLuint *a){
	glDeleteBuffers(1, a);
}

