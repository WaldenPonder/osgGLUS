uniform sampler2D baseTexture;
varying vec2 texcoord;

float rgb2gray(vec3 color) {
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

void main()
{	
    vec4 result = texture( baseTexture, texcoord);

    //result = pow(result, vec4(1.0 / 2.2));
    
    //if(result.r + result.g + result.b < 1e-3) discard;
    
    gl_FragColor =  vec4(result.rag, .5 * rgb2gray(result.rgb)); 
}


