#version 330 core

const vec4 verts[3] = vec4[3](
	vec4(0.f, 0.5f, 0.f, 1.f),
	vec4(0.5f, -0.5f, 0.f, 1.f),
	vec4(-0.5f, -0.5f, 0.f, 1.f)
);

layout(location = 0) in mat4 model;

void main(void){
	gl_Position = model * verts[gl_VertexID];
}

