varying vec3 WorldPos;
varying vec3 Normal;
varying vec3 camPos;

void main()
{	
	WorldPos = gl_Vertex.xyz;
	//Normal = gl_Normal;
	Normal = gl_NormalMatrix * gl_Normal;
	camPos = mat3(gl_ModelViewMatrix) * vec3(0);
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}