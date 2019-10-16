
varying vec3 vNormal;
varying vec3 vViewPosition;
varying vec4 vColor;
varying vec2 texcoord;
//uniform mat4 u_MVP;

void main()
{	
	vNormal   = normalize(gl_NormalMatrix * gl_Normal);
    vColor    = gl_Color;
    vec4 mvPosition = gl_ModelViewMatrix * gl_Vertex;
	vViewPosition   = mvPosition.xyz;	
    texcoord = gl_MultiTexCoord0.st;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}