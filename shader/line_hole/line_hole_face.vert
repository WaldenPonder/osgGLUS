#version 430 compatibility
layout (location=0) in vec3 a_pos;
layout (location=1) in vec4 a_color;
layout (location=2) in uint a_id;

uniform mat4 u_MVP;

out vec4 v_color;
flat out uint v_id;

void main()
{
   v_id = a_id;
   v_color = a_color;
   gl_Position = u_MVP * vec4(a_pos,1.0);
}
