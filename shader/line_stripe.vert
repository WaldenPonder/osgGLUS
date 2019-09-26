#version 430 
layout (location=0) in vec3 a_pos;
layout (location=1) in vec2 a_index;

uniform mat4 u_MVP;
//uniform mat4  osg_ModelViewProjectionMatrix;

flat out float v_index_start;
flat out float v_index_end;

void main()
{
   v_index_start = a_index[0];
   v_index_end = a_index[1];
   
   gl_Position = u_MVP * vec4(a_pos,1.0);
}