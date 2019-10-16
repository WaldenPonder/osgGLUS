uniform sampler2D baseTexture;
varying vec2 texcoord;


void main()
{	
    vec4 result = texture( baseTexture, texcoord);

    result = pow(result, vec4(1.0 / 1.1));
    
    //if(result.r + result.g + result.b < 1e-6) discard;
    
    gl_FragColor =  result; 
}


