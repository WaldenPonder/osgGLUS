precision highp float;

uniform sampler2D baseTexture;
varying vec2 texcoord;

uniform float u_screen_width, u_screen_height;
uniform float u_gradientThreshold;
uniform vec3 u_color;

float rgb2gray(vec3 color) {
    
   // return (color.r + color.g + color.b) / 3.0;
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

// in vec2 texcoord;
float pixel_operator(float dx, float dy) {
    vec4 rgba_color = texture( baseTexture, texcoord + vec2(dx,dy) );
	
	//if(rgba_color.a < .01)
	//   rgba_color = vec4(1., 1., 1., 1.);
	//else 
	//   rgba_color = vec4(1., 1., 1., 1.);
	   	
    return rgb2gray(rgba_color.rgb);
}

float sobel_filter()
{
    float dx = 1.0 / float(u_screen_width);
    float dy = 1.0 / float(u_screen_height); 

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
    //vec4 c = texture( baseTexture, texcoord);
    //gl_FragColor =  c;

    //return;

    float dist = sobel_filter();
    gl_FragColor =  vec4(u_color * dist, 1.0); 
    //return;
/*     //*************************边缘检测伐值（越小出来的轮廓越多）**************************************************  
    if(dist > u_gradientThreshold)
    {
    //**************************轮廓颜色*************************************************
        gl_FragColor =  vec4(u_color, 1);     
    } 
    else
       gl_FragColor = vec4(0, 0, 0, 1);   */
}


