#version 150
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in Varing
{
	vec3 pos_;
	vec3 center_;
} var[];

void main() 
{  
    vec4 p1 = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
	vec4 p2 = gl_in[1].gl_Position / gl_in[1].gl_Position.w;
	vec4 p3 = gl_in[2].gl_Position / gl_in[2].gl_Position.w;

	gl_Position = gl_in[0].gl_Position; 
	EmitVertex(); 
	
	gl_Position = gl_in[1].gl_Position; 
	EmitVertex(); 

	gl_Position = gl_in[2].gl_Position; 
	EmitVertex(); 

	EndPrimitive();
}