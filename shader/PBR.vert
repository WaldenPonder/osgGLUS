varying vec3 Normal;
varying vec4 posInViewSpace;

void main()
{
	Normal   = normalize(gl_NormalMatrix*gl_Normal);
    posInViewSpace = gl_ModelViewMatrix*gl_Vertex;
	
	//vec3 pos = gl_Vertex.xyz + vec3(1);
	//lightDir = normalize(u_lightPosition - posInViewSpace.xyz);
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;//ftransform();
}