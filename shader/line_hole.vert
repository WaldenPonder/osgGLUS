#version 430 
layout (location=0) in vec3 a_pos;
layout (location=1) in vec3 a_color;
layout (location=2) in vec3 a_id;

uniform mat4 u_MVP;
//uniform mat4  osg_ModelViewProjectionMatrix;

flat out vec3 v_id;
out vec3 v_color;
void main()
{
    v_id = a_id;
	v_color = a_color;
   gl_Position = u_MVP * vec4(a_pos,1.0);
}
