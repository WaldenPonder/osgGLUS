uniform sampler2D baseTexture;

varying vec3 vNormal; 
varying vec3 vViewPosition; 
varying vec4 vColor;
varying vec2 texcoord;

uniform float u_screen_width, u_screen_height;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

float rgb2gray(vec3 color) {
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

// in vec2 texcoord;
float pixel_operator(float dx, float dy) {
    vec4 rgba_color = texture( baseTexture, texcoord + vec2(dx,dy) );
    return rgb2gray(rgba_color.rgb);
}

float sobel_filter()
{
    float dx = 1.0 / float(u_screen_width); // e.g. 1920
    float dy = 1.0 / float(u_screen_height); // e.g. 1080

    float s00 = pixel_operator(-dx, dy);
    float s10 = pixel_operator(-dx, 0.0);
    float s20 = pixel_operator(-dx, -dy);
    float s01 = pixel_operator(0.0, dy);
    float s21 = pixel_operator(0.0, -dy);
    float s02 = pixel_operator(dx, dy);
    float s12 = pixel_operator(dx, 0.0);
    float s22 = pixel_operator(dx, -dy);
    
    float sx = s00 + 2.0 * s10 + s20 - (s02 + 2.0 * s12 + s22);
    float sy = s00 + 2.0 * s01 + s02 - (s20 + 2.0 * s21 + s22);
    float dist = sx * sx + sy * sy;
    
    return dist;
}

void main()
{	
   vec4 texColor = texture2D( baseTexture, texcoord);
   //gl_FragColor =  texColor;
   //return;
    // predefine var
    float gradientThreshold = 0.01;    
    float graylevel = sobel_filter();
    
    if (graylevel > gradientThreshold)
    {
	   vec2 tex_offset = 1 / textureSize(baseTexture, 0); // gets size of single texel
        vec3 result = vec4(1, 0, 0, 1) * weight[0]; // current fragment's contribution
		if(1)
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
	
        gl_FragColor =  texColor;//vec4(result, 1);
    } 
    else 
    {
	   //discard;
       gl_FragColor = vec4(0, 0, 0, 1);
    }
}


