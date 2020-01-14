varying vec3 WorldPos;
uniform samplerCube env_cube_texture;

void main()
{		
	vec3 c = texture(env_cube_texture, WorldPos).rgb ;
	c = pow(c, vec3(1.0/2.2)); 
    gl_FragColor = vec4(c, 1.0);
}