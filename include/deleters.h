#ifndef DELETERS_H
#define DELETERS_H

#include "gl_core_3_3.h"

namespace detail {
	/*
	 * Various functions to free OpenGL resources to be given to
	 * our shared/unique pointers
	 */
	void delete_buffer(GLuint *b);
	void delete_vao(GLuint *a);
}


#endif

