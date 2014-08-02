#version 330 core

uniform sampler2DArray tile_atlas;

out vec4 color;

in vec3 fnormal;
in vec3 fuv;

void main(void){
	color = texture(tile_atlas, fuv);
}

