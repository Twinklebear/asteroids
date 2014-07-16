#version 330 core

uniform sampler2D tile_atlas;

out vec4 color;

in vec3 fnormal;
in vec2 fuv;

void main(void){
	color = texture(tile_atlas, fuv);
}

