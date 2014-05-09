#version 330 core

layout(std140) uniform Test {
	mat4 m;
	float f;
	float arr[10];
	int b;
};

layout(location = 0) in vec3 pos;

void main(void){
	gl_Position = m * vec4(pos, 1.f);
}

