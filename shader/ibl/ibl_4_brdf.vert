varying vec3 WorldPos;
//gl_FragColor
varying vec2 TexCoords;

void main()
{	
	WorldPos = gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}