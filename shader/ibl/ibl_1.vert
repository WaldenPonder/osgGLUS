varying vec3 WorldPos;

void main()
{	
	WorldPos = gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}