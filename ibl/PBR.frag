uniform vec4 albedo;                    
uniform vec3 CameraPos;  

uniform float metallic;
uniform float roughness;
uniform float ambient_factor;
varying vec4 WorldPos; 
varying vec3 Normal; 
uniform bool selected;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

void main()
{
   vec4 ALBEDO = albedo;
   if(selected)
   {
      ALBEDO = ALBEDO * .3 + vec4(.0, .5, .5, .0);
   }
   
   if(!gl_FrontFacing) Normal = -Normal;
   vec3 N = normalize(Normal);
   vec3 V = normalize(CameraPos - WorldPos.xyz);
   
   vec3 F0 = vec3(0.04);
   F0 = mix(F0, ALBEDO.rgb, metallic);
   
   vec3 Lo = vec3(0.0);
  // vec3 lightPositions = vec3(0, 0, 1000);
   
   vec3 L = normalize(vec3(1.0));
   vec3 H = normalize(V + L);
   
  // float distance = length(lightPositions - WorldPos);
  // float attenuation  = 1.0 / (distance * distance);
   
   vec3 radiance = vec3(2.0);
   
   float NDF = DistributionGGX(N, H, roughness);
   float G = GeometrySmith(N, V, L, roughness);
   vec3 F = fresnelSchlick(max(dot(H,V), 0.0), F0);
   
   vec3 kS = F;
   vec3 kD = vec3(1.0) - kS;
   kD *= (1.0 - metallic);
   
   vec3 nominator = NDF * G * F;
   float denominator = 4.0 * max(0.0, dot(N, V)) * max(0.0, dot(N, L)) + 0.001;
   vec3 specular = nominator / denominator;
   
   float NdotL = max(dot(N, L), 0.0);
   Lo += (kD * ALBEDO.rgb /PI + specular) * radiance * NdotL;
   
   vec3 ambient = vec3(ambient_factor) * ALBEDO.rgb; 
   vec3 color = ambient + Lo;
   
   color = color / (color + vec3(1.0));
   color = pow(color, vec3(1.0/2.2));
   
   gl_FragColor = vec4(color, 1.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  