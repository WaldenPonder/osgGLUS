#version 430 compatibility
layout (lines) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
    flat int v_id;
    vec4 v_color;
} gs_in[];


out vec4 g_color;
flat out int g_id;
flat out vec4 v_linePt;

void main() 
{  
#if 0
   //透视
    vec4 pos1 = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
	vec4 pos2 = gl_in[1].gl_Position / gl_in[1].gl_Position.w;

	// vec4 pos1 = (p1 + vec4(1.0)) / 2.0;
	// vec4 pos2 = (p2 + vec4(1.0)) / 2.0; 
#else
	//正交视图
    vec4 pos1 = gl_in[0].gl_Position;
	vec4 pos2 = gl_in[1].gl_Position;
#endif

	
    v_linePt.xy = pos1.xy;
	v_linePt.zw = pos2.xy;
	//v_linePt = vec4(222, 0., 0., 0.);
	g_color = gs_in[0].v_color;
	g_id = gs_in[0].v_id;
	gl_Position = gl_in[0].gl_Position; 
	EmitVertex(); 
	
	v_linePt.xy = pos1.xy;
	v_linePt.zw = pos2.xy;
	//v_linePt = vec4(222, 0., 0., 0.);
	g_color = gs_in[1].v_color;
	g_id = gs_in[1].v_id;
	gl_Position = gl_in[1].gl_Position; 
	EmitVertex(); 
	
	EndPrimitive();
}