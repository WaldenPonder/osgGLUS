#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 windowMat;

in Varing
{
	vec3 pos_;
	vec3 center_;
} var[];

out vec3 pos_;
out vec3 center_;
//out vec3 color_;

void main() 
{  
    vec4 p1 = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
	vec4 p2 = gl_in[1].gl_Position / gl_in[1].gl_Position.w;
	vec4 p3 = gl_in[2].gl_Position / gl_in[2].gl_Position.w;
	
    vec4 pos1 = (windowMat * p1);
	vec4 pos2 = (windowMat * p2);
	vec4 pos3 = (windowMat * p3);
	
	float val1 = max(abs((pos1 - pos2).x), abs((pos1 - pos2).y));
	float val2 = max(val1, max(abs((pos1 - pos3).x), abs((pos1 - pos3).y)));
	float val3 = max(val2, max(abs((pos3 - pos2).x), abs((pos3 - pos2).y)));
	
	//小于2个像素不显示
	if(val3 <= 2.0) return;
	
	/*if(pos1.x < 1000)
	  pos1 = vec4(1, 0, 0, 0);
	 else 
	  pos1 = vec4(0, 1, 0, 0);
	  */
    pos_ = var[0].pos_;
	center_ = var[0].center_;
	//color_ = vec3(pos1.xy , 0);
	gl_Position = gl_in[0].gl_Position; 
	EmitVertex(); 
	
	pos_ = var[1].pos_;
	center_ = var[1].center_;
	//color_ = vec3(pos1.xy , 0);
	gl_Position = gl_in[1].gl_Position; 
	EmitVertex(); 
	
	pos_ = var[2].pos_;
	center_ = var[2].center_;
	//color_ = vec3(pos1.xy , 0);
	gl_Position = gl_in[2].gl_Position; 
	EmitVertex(); 

	EndPrimitive();
}