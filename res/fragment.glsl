#version 330 core

//Colors to use
const vec4 colors[3] = vec4[3](
	vec4(1, 0, 0, 1), vec4(0, 1, 0, 1), vec4(0, 0, 1, 1)
);

out vec4 color;

in vec3 fnormal;
in vec2 fuv;
flat in int fcolor_idx;

void main(void){
	color = colors[fcolor_idx] * vec4((fnormal + 1) / 2.f, 1.f);
}

