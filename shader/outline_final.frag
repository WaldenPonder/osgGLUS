uniform sampler2D baseTexture;
varying vec2 texcoord;
uniform float u_alpha_factor;

/* float rgb2gray(vec3 color) {
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
} */

void main()
{	
    vec4 result = texture( baseTexture, texcoord);
    //***********************轮廓透明度系数***************************************
    
   gl_FragColor =  vec4(result.rgb, u_alpha_factor *  max(result.b,max(result.r, result.g))); 
    //gl_FragColor = result;//vec4(result.rgb, .5 * rgb2gray(result.rgb));
}


