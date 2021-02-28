#version 430 core
layout (lines) in;
layout (line_strip, max_vertices = 2) out;
uniform mat4 windowMat;

in VS_OUT {
    flat vec3 v_id;
    vec3 v_color;
} gs_in[];


out vec3 g_color;
flat out vec3 g_id;
flat out vec4 v_linePt;

void main() 
{  
    vec4 p1 = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
	vec4 p2 = gl_in[1].gl_Position / gl_in[1].gl_Position.w;
	
  //  vec4 pos1 = (windowMat * p1);
	//vec4 pos2 = (windowMat * p2); 

	//pos1.xy /= vec2(1920.0, 1920.0);
	//pos2.xy /= vec2(1920.0, 1920.0);
	
		
	vec4 pos1 = (p1 + vec4(1.0)) / 2.0;
	vec4 pos2 = (p2 + vec4(1.0)) / 2.0; 

	
    v_linePt.xy = pos1.xy;
	v_linePt.zw = pos2.xy;
	//v_linePt = vec4(1,1,0,1);
	g_color = gs_in[0].v_color;
	g_id = gs_in[0].v_id;
	gl_Position = gl_in[0].gl_Position; 
	EmitVertex(); 
	
	v_linePt.xy = pos1.xy;
	v_linePt.zw = pos2.xy;
	//v_linePt = vec4(1,0,0,1);
	g_color = gs_in[1].v_color;
	g_id = gs_in[1].v_id;
	gl_Position = gl_in[1].gl_Position; 
	EmitVertex(); 
	
	EndPrimitive();
}