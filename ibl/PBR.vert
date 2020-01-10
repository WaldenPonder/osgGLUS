varying vec3 Normal;
varying vec4 WorldPos;

void main()
{
	Normal   = normalize(gl_NormalMatrix*gl_Normal);
    WorldPos = gl_ModelViewMatrix*gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;//ftransform();
}