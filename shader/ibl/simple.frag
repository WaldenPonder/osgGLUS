precision highp float;

uniform sampler2D baseTexture;
varying vec2 texcoord;

void main()
{	
    vec4 c = texture( baseTexture, texcoord);
    gl_FragColor =  c;
}


