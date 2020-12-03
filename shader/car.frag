precision highp float;

uniform sampler2D baseTexture;
uniform sampler2D specularTexture;
varying vec2 texcoord;

void main()
{	
    vec4 c = texture( baseTexture, texcoord);
	vec4 c2 = texture( specularTexture, texcoord);
    gl_FragColor =  1.5 * c * c2;
}


