#version 330 core
layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aNormal;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec2 aRandom;
layout (location = 3) in vec3 aRootPos;

out vec2 texCoords;
//out vec3 normal;
out vec4 wolrdPos;
/*
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
*/
uniform float osg_FrameTime;
uniform mat4 osg_ModelViewProjectionMatrix;

//uniform float time;
uniform float localWindIntensity;
uniform float localWindSpeed;
uniform float globalWindIntensity;
uniform float globalWindSpeed;
uniform float globalWindPhase;
uniform vec2  planeDimension;
uniform vec2  windVector;

const float TWO_PI  = 6.2831852f;
const float delta   = 1e-4;
vec2  windDir       = normalize(windVector);


vec3 hash(vec2 p1 ,vec2 p2,  vec2 p3) 
{
    vec2 c=vec2(15.32f,5.78f);
    vec3 v=vec3(dot(p1,c), dot(p2,c), dot(p3,c));	
    return fract(sin(v) * 43758.236237153);
}

mat3x2 hash3(vec2 p1,vec2 p2,vec2 p3)
{
    mat3x2 m  = mat3x2(p1,p2,p3);
    mat3x2 m1 = m*0.754;
	mat3x2 m2 = m*1.5743;

    vec3 h1 = hash(m1[0],m1[1],m1[2]);
	vec3 h2 = hash(m2[0].yx+4.5891, m2[1].yx+4.5891, m2[2].yx+4.5891);
	
    mat3x2 mt;
	mt[0]= vec2(h1.x,h2.x)-0.5f;
	mt[1]= vec2(h1.y,h2.y)-0.5f;
	mt[2]= vec2(h1.z,h2.z)-0.5f;
	
	return mt;
}

// Gabor/Voronoi mix 3x3 kernel
float gavoronoi3( vec2 p)
{    
    vec2  ip  = floor(p);
    vec2  fp  = fract(p);
    vec2  dir = windDir; 
    float f   = TWO_PI;     //frequency
    float v   = 0.4f;      //cell variability <1.
    float dv  = .4;        //direction variability <1.
    float va  = 0.0;
   	float wt  = 0.0;
	
    for (int i=-1; i<=1; i++) 
	{		
		vec2 o1 = vec2(i,-1)-0.5f;
		vec2 o2 = vec2(i,0 )-0.5f;
		vec2 o3 = vec2(i,1 )-0.5f;
		
        mat3x2 m  = hash3(ip-o1,ip-o2,ip-o3);
		mat3x2 mp = mat3x2(fp+o1,fp+o2,fp+o3);
		mp-=m;
		
        vec3 d = vec3(dot(mp[0], mp[0]),dot(mp[1],mp[1]),dot(mp[2],mp[2]));	
        vec3 w = exp(-d*4.);
		
        wt= wt+w.x+w.y+w.z;

        m*=dv;
		
        vec2 h1 = m[0]+dir;
	    vec2 h2 = m[1]+dir;
	    vec2 h3 = m[2]+dir;
		
	    vec3 v = vec3(dot(mp[0],h1),dot(mp[1],h2),dot(mp[2],h3))*TWO_PI/v;
	    vec3 cs = cos(v)*w;
	    
	    va = va+cs.x+cs.y+cs.z;
	}
    
    return va/wt;
}

vec2 hash( vec2 x )
{
    const vec2 k = vec2( 0.3183099, 0.3678794 );
    x = x*k + k.yx;
    return -1.0 + 2.0*fract( 16.0 * k*fract( x.x*x.y*(x.x+x.y)) );
}

float noise( in vec2 p )
{
    vec2 i = floor( p );
    vec2 f = fract( p );
	
	vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot( hash( i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ), 
                     dot( hash( i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( hash( i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ), 
                     dot( hash( i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}

void main()
{
 	
	vec3  pos=aPos;

	float time = osg_FrameTime * .5;

    float localFrq  = time*localWindSpeed;
    float globalFrq = time*globalWindSpeed;
	//aa
	vec2  invPlaneDimension = 10.0/vec2(planeDimension.x,planeDimension.y); 
	
	vec2  pLocal  = pos.xz*invPlaneDimension*aRandom.xy-localFrq*aRandom.xy;
	vec2  pGlobal = pos.xz*invPlaneDimension*globalWindPhase+globalFrq;
	float b = noise(pLocal+vec2(0,-delta));
	float t = noise(pLocal+vec2(0,delta));
	float l = noise(pLocal+vec2(-delta,0));
	float r = noise(pLocal+vec2(delta,0));

	vec2 localWind  = vec2(b-t,r-l)/delta;
	vec2 globalWind = vec2(gavoronoi3(pGlobal));
	globalWind *= globalWindIntensity;
    localWind  *= localWindIntensity;

    vec2 wind=(localWind+globalWind*windDir)*pow(aTexCoords.y,2)*planeDimension*0.1f;
	float lengthOri2=dot(aPos-aRootPos,aPos-aRootPos);
	float maxXZOffset2=lengthOri2-aPos.y*aPos.y;
	wind=clamp(dot(wind,wind)/maxXZOffset2,0.3f,1.0f)*wind;
    pos.xy+= wind;

	float xzOffset=(pos.x-aRootPos.x)*(pos.x-aRootPos.x) + (pos.z-aRootPos.z)*(pos.z-aRootPos.z);
	///pos.y -= length(wind)*0.8*pow(aTexCoords.y,2);//sqrt(max(lengthOri2-xzOffset,0.0f))+aRootPos.y;
	
	
    //normal      = aNormal;
    texCoords   = aTexCoords.xy;
    //wolrdPos    = model*vec4(pos,1.0f);  
    gl_Position = osg_ModelViewProjectionMatrix * vec4(pos,1.0f);
	
}




