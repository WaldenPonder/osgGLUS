#version 430 compatibility
layout (location=0) in vec3 a_pos;
layout (location=1) in vec4 a_color;
layout (location=2) in uint a_id;

uniform mat4 u_MVP;
//uniform mat4  osg_ModelViewProjectionMatrix;


out VS_OUT {
	flat uint v_id;
    vec4 v_color;
} gs_in;

void main()
{
   gs_in.v_id = a_id;
   gs_in.v_color = a_color;
   gl_Position = u_MVP * vec4(a_pos,1.0);
}
