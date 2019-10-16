uniform sampler2D baseTexture;

varying vec3 vNormal; 
varying vec3 vViewPosition; 
varying vec4 vColor;
varying vec2 texcoord;

uniform float u_screen_width, u_screen_height;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
uniform bool u_is_horizontal;

float rgb2gray(vec3 color) {
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

void main()
{	
    //vec3 Kd = vColor.rgb;
    vec4 texColor = texture( baseTexture, texcoord);
   // gl_FragColor = vec4(0,0,1,1);
    //return;
 
    vec2 tex_offset = 1.0 / textureSize(baseTexture, 0); // gets size of single texel
    vec3 result = texColor * weight[0]; // current fragment's contribution
    if(u_is_horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(baseTexture, texcoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(baseTexture, texcoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(baseTexture, texcoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(baseTexture, texcoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }  

    //result = pow(result, vec3(1.0 / 1.1));
    
    //if(result.r + result.g + result.b < 1e-3) discard;
    
    gl_FragColor =  vec4(result, texColor.a); 
}


