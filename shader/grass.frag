#version 330 core
out vec4 FragColor;

in vec2 texCoords;
//in vec3 normal;
in vec4 worldPos;

uniform sampler2D diffuseTex;

void main()
{
    vec4 diffuse=texture(diffuseTex,texCoords);   
	
	diffuse.rgb = pow(diffuse.rgb, vec3(1.0/2.2)); 
	
    FragColor.rgb = 1.3 * diffuse.rgb;
	FragColor.a=diffuse.a;
	
	//FragColor = vec4(1, 0, 0, 1);
}