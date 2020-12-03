
uniform sampler2D baseTexture;
//gis cull
uniform sampler2DShadow u_GISCullTexture1;
uniform sampler2DShadow u_GISCullTexture2;
uniform sampler2DShadow u_GISCullTexture3;

uniform float u_zgis_CULL1;
uniform float u_zgis_CULL2;
uniform float u_zgis_CULL3;

varying vec2 texcoord;
varying vec4 posInGISRegionSpace1;
varying vec4 posInGISRegionSpace2;
varying vec4 posInGISRegionSpace3;


//饱和度和亮度系数
const float faturation = 1.5;
const float brightness = 1.5;

float fTexelSize= 1.41 / 4096.0;
float fZOffSet  = -0.001954;
   
//----------------------------------------------------------------------------my_sample
float my_sample( sampler2DShadow TEXTURE, vec4 sc )
{   
#if 1
    //gaussian
    float shadow = 0.0;                                                                                                     
	for(int i = -2; i <= 2; ++i)                                                                                             
	{                                                                                                                         
		for(int j = -2; j <= 2; ++j)                                                                                          
		{                                                                                                                 
			shadow += textureProjOffset(TEXTURE, sc, ivec2(i, j)) * 0.04;                    
		}                                                                                                                 
	}                                                                                                                       
	                                                                                                                       
	return shadow;                                                                     
#else
   //return 1;
	ivec3 offsetCoord;
	offsetCoord.xy = ivec2( mod( gl_FragCoord.xy, OffsetTexSize.xy ) );

	float sum = 0.0, shadow = 1.0;
	int samplesDiv2 = int(OffsetTexSize.z);
	
	float R = Radius * 1;

	// Don't test points behind the light source.
	if( sc.z >= 0 )
	{
		for( int i = 0 ; i < 4; i++ ) {
			offsetCoord.z = i;
			vec4 offsets = texelFetch(u_shadow_map_Jittering,offsetCoord,0) * R * sc.w;

			sc.xy = sc.xy + offsets.xy;
			sum += textureProj(TEXTURE, sc);
			sc.xy = sc.xy + offsets.zw;
			sum += textureProj(TEXTURE, sc);
		}
		shadow = sum / 8.0;

		if( shadow != 1.0 && shadow != 0.0 ) {
			for( int i = 4; i < samplesDiv2; i++ ) {
				offsetCoord.z = i;
				vec4 offsets = texelFetch(u_shadow_map_Jittering, offsetCoord,0) * R * sc.w;

				sc.xy = sc.xy + offsets.xy;
				sum += textureProj(TEXTURE, sc);
				sc.xy = sc.xy + offsets.zw;
				sum += textureProj(TEXTURE, sc);
			}
			shadow = sum / float(samplesDiv2 * 2.0);
		}
	}
	
	return shadow;
#endif

   float shadowOrg1 = textureProj(TEXTURE, sc + vec4(0.0,0.0,fZOffSet, 0));
   float shadow01 = textureProj(TEXTURE, sc + vec4(-fTexelSize,-fTexelSize,fZOffSet, 0));
   float shadow11 = textureProj(TEXTURE, sc + vec4(fTexelSize,-fTexelSize,fZOffSet, 0));
   float shadow21 = textureProj(TEXTURE, sc + vec4(fTexelSize,fTexelSize,fZOffSet, 0));
   float shadow31 = textureProj(TEXTURE, sc + vec4(-fTexelSize,fTexelSize,fZOffSet, 0));
   float shadow1 = (2 * shadowOrg1 + shadow01 + shadow11 + shadow21 + shadow31) / 6.0;	
    
   return shadow1;
}


//----------------------------------------------------------------------------gis_cull
bool gis_cull(inout vec4 color)
{			
	// select the shadow map : split
	float testZ = gl_FragCoord.z * 2.0 - 1.0;
	
	float map1 = step(testZ, u_zgis_CULL1);
	float map2 = step(u_zgis_CULL1, testZ) * step(testZ, u_zgis_CULL2);
	float map3 = step(u_zgis_CULL2, testZ) * step(testZ, u_zgis_CULL3);
		
#if 0
	float shadow1 = textureProj(u_GISCullTexture1, posInGISRegionSpace1); 
	float shadow2 = textureProj(u_GISCullTexture2, posInGISRegionSpace2); 
	float shadow3 = textureProj(u_GISCullTexture3, posInGISRegionSpace3); 
#else
	float shadow1 = my_sample(u_GISCullTexture1, posInGISRegionSpace1); 
	float shadow2 = my_sample(u_GISCullTexture2, posInGISRegionSpace2); 
	float shadow3 = my_sample(u_GISCullTexture3, posInGISRegionSpace3); 
#endif
	
	float term1 = (1.0 - shadow1) * map1;
	float term2 = (1.0 - shadow2) * map2;
	float term3 = (1.0 - shadow3) * map3;
		  
	float v = clamp(term1 + term2 + term3, 0, 1);
	//color.rgb  = vec3(map1, map2, map3);
	color.a = 1 - v;//1 - v;//vec3(0, shadow2, 0);
	//color.rgb = color.a;
	
	//if(v != 0)
	//  discard;
	return false;
}


void main()
{	
	vec4 color = texture( baseTexture, texcoord);
	if(gis_cull(color)) return;

    
	color.rgb = brightness * color.rgb;
			
	float gray = 0.2125 * color.r + 0.7154 * color.g + 0.0721 * color.b;
	vec3 grayColor = vec3(gray);	
	color.rgb = mix(grayColor, color.rgb, faturation);
	
    gl_FragColor =  color;
}


