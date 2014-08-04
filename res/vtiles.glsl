#version 330 core

uniform samplerBuffer uvs;

layout(std140) uniform Viewing {
	mat4 view, proj;
};

layout(location = 0) in vec3 pos;
layout(location = 3) in mat4 model;
layout(location = 7) in int tile_id;

out vec2 fuv;

void main(void){
	fuv = texelFetch(uvs, 4 * tile_id + gl_VertexID).xy;
	gl_Position = proj * view * model * vec4(pos, 1.f);
}

