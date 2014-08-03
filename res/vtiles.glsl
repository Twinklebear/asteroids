#version 330 core

uniform samplerBuffer uvs;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 3) in mat4 model;
layout(location = 7) in int tile_id;

out vec3 fnormal;
out vec3 fuv;

void main(void){
	fnormal = normal;
	fuv = texelFetch(uvs, 4 * tile_id + gl_VertexID).xyz;
	gl_Position = model * vec4(pos, 1.f);
}

