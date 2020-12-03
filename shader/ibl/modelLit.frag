varying vec3 WorldPos;

uniform sampler2D baseTexture;

varying vec3 vNormal; 
varying vec3 vViewPosition; 
varying vec4 vColor;
varying vec2 texcoord;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}


void main()
{	
    vec4 texColor = texture2D( baseTexture, texcoord);
	vec2 uv = SampleSphericalMap(normalize(vNormal));
    vec3 Kd = vColor.rgb;// * texture(equirectangularMap, uv).rgb;
	
	//无纹理
	if(texColor[0] + texColor[1] + texColor[2] < .001)
	{
       	//texColor = vec4(Kd, 1);
			
		vec3 normal1  = vNormal;	
		vec3 LightPosition=vec3(5,4,10);			
		vec3 s = normalize(LightPosition- vViewPosition);

		vec3 LightPosition2=vec3(-5,-4,10);			
		vec3 s2 = normalize(LightPosition2- vViewPosition);	

		float c1=max( dot(s, normal1),dot(s2,normal1));	 

		vec3 diffuse  = Kd*clamp( c1,0.0,1.0);	
		texColor = vec4(diffuse, 1);
		texColor.rgb = normal1;
		//texColor = vec4(1,0,0,1);
	} 
	else
	{
	   texColor = 1 * vec4(Kd, 1) * texColor;
	   //texColor = vec4(0,1,0,1);
	}
		
  	gl_FragColor = texColor;
	
}
