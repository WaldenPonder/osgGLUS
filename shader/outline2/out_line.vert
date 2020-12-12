#version 150
void main()
{	
	//gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	vec4 pos = gl_ModelViewProjectionMatrix * gl_Vertex;
	vec3 normal = normalize(gl_NormalMatrix * gl_Normal);  
	vec4 n = gl_ModelViewProjectionMatrix * vec4(gl_Normal, 1);
	pos.xy += 0.01 * n.xy;
	gl_Position = pos;
}
