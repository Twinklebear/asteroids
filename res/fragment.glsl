#version 330 core

out vec4 color;

in vec3 fnormal;
in vec2 fuv;

void main(void){
	color = vec4((fnormal + 1) / 2.f, 1.f);
}

