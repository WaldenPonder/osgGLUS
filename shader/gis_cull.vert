uniform mat4 u_GISCullRegionMat1; 
uniform mat4 u_GISCullRegionMat2; 
uniform mat4 u_GISCullRegionMat3; 

varying vec2 texcoord;

varying vec4 posInGISRegionSpace1;
varying vec4 posInGISRegionSpace2;
varying vec4 posInGISRegionSpace3;

void main()
{	
    texcoord = gl_MultiTexCoord0.st;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	
	vec4 v = gl_Vertex;
	posInGISRegionSpace1 =  u_GISCullRegionMat1   * gl_ModelViewMatrix * v;
	posInGISRegionSpace1 = posInGISRegionSpace1* 0.5 + 0.5;
	
	posInGISRegionSpace2 =  u_GISCullRegionMat2  * gl_ModelViewMatrix * v;
	posInGISRegionSpace2 = posInGISRegionSpace2* 0.5 + 0.5;
	
	posInGISRegionSpace3 =  u_GISCullRegionMat3  * gl_ModelViewMatrix * v;
	posInGISRegionSpace3 = posInGISRegionSpace3* 0.5 + 0.5;
}