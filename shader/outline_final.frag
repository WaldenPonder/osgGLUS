uniform sampler2D baseTexture;
varying vec2 texcoord;

void main()
{	
    vec4 result = texture( baseTexture, texcoord);
   gl_FragColor =  result; 
}


