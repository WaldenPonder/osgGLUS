varying vec3 WorldPos;

void main()
{	
	WorldPos = gl_Vertex.xyz;
	vec4 clip = gl_ProjectionMatrix * mat4(mat3(gl_ModelViewMatrix)) * gl_Vertex;
	gl_Position = clip.xyzw;
	//gl_Position.w = gl_Position.z;
}