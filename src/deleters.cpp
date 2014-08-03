#include "deleters.h"
#include "gl_core_3_3.h"

void detail::delete_buffer(GLuint *b){
	glDeleteBuffers(1, b);
}
void detail::delete_vao(GLuint *a){
	glDeleteBuffers(1, a);
}
void detail::delete_texture(GLuint *t){
	glDeleteTextures(1, t);
}

