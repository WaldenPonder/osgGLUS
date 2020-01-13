varying vec3 WorldPos;
varying vec3 Normal;

void main()
{	
	WorldPos = gl_Vertex;
	Normal = gl_Normal;
	
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}