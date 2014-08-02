#version 330 core

layout(std140) uniform TileUVs {
	vec2 uvs[16];
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 3) in mat4 model;
layout(location = 7) in int tile_id;

out vec3 fnormal;
out vec2 fuv;

void main(void){
	fnormal = normal;
	fuv = uvs[4 * tile_id + gl_VertexID];
	gl_Position = model * vec4(pos, 1.f);
}

