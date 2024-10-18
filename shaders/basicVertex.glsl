#version 330 core
layout (location = 0) in vec3 pos;
uniform mat4 m;
uniform mat4 vp;
void main(){
	gl_Position = vp * m * vec4(pos, 1);
}