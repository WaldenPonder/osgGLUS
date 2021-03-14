#version 430 compatibility
layout (location=0) in vec3 a_pos;
layout (location=1) in vec4 a_color;
layout (location=2) in vec4 a_id;

uniform mat4 u_MVP;
//uniform mat4  osg_ModelViewProjectionMatrix;


out VS_OUT {
	flat int v_id;
    vec4 v_color;
} gs_in;

void main()
{
   gs_in.v_id = int(a_id.x);
   gs_in.v_color = a_color;
   
   	// if(a_id.x == 1)  gs_in.v_color = vec4(1,0,0,1);
	// else if(a_id.x == 2) 
	// gs_in.v_color = vec4(1,1,0,1);
	
   gl_Position = u_MVP * vec4(a_pos,1.0);
}
