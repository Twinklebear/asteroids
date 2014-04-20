#version 330 core

layout(std140) uniform Viewing {
	mat4 view, proj;
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in mat4 model;

out vec3 fnormal;
out vec2 fuv;

void main(void){
	fnormal = normal;
	fuv = uv;
	gl_Position = proj * view * vec4(pos, 1.f);
}

