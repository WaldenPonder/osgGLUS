#version 430 compatibility

layout (location=0) in vec3 a_pos;
layout (location=1) in vec2 a_uv;
uniform mat4 u_MVP;
out vec2 texcoord;

void main()
{	
	texcoord = a_uv;
	gl_Position = u_MVP * vec4(a_pos,1.0);
}