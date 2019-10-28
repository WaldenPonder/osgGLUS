#version 430 
layout (location=0) in vec3 a_pos;

uniform mat4 u_MVP;
//uniform mat4  osg_ModelViewProjectionMatrix;

out VS_OUT {
    int vertexID;
} vs_out;


void main()
{  
   vs_out.vertexID = gl_VertexID;
   gl_Position = u_MVP * vec4(a_pos,1.0);
}