uniform sampler2D baseTexture;

varying vec2 texcoord;

uniform float u_screen_width, u_screen_height;
//uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
uniform vec2 u_dir;

vec4 blur5(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.3333333333333333) * direction;
  color += texture2D(image, uv) * 0.29411764705882354;
  color += texture2D(image, uv + (off1 / resolution)) * 0.35294117647058826;
  color += texture2D(image, uv - (off1 / resolution)) * 0.35294117647058826;
  return color; 
}
/*
 vec4 blur9(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.3846153846) * direction;
  vec2 off2 = vec2(3.2307692308) * direction;
  color += texture2D(image, uv) * 0.2270270270;
  color += texture2D(image, uv + (off1 / resolution)) * 0.3162162162;
  color += texture2D(image, uv - (off1 / resolution)) * 0.3162162162;
  color += texture2D(image, uv + (off2 / resolution)) * 0.0702702703;
  color += texture2D(image, uv - (off2 / resolution)) * 0.0702702703;
  return color;
} */

vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.411764705882353) * direction;
  vec2 off2 = vec2(3.2941176470588234) * direction;
  vec2 off3 = vec2(5.176470588235294) * direction;
  color += texture2D(image, uv) * 0.1964825501511404;
  color += texture2D(image, uv + (off1 / resolution)) * 0.2969069646728344;
  color += texture2D(image, uv - (off1 / resolution)) * 0.2969069646728344;
  color += texture2D(image, uv + (off2 / resolution)) * 0.09447039785044732;
  color += texture2D(image, uv - (off2 / resolution)) * 0.09447039785044732;
  color += texture2D(image, uv + (off3 / resolution)) * 0.010381362401148057;
  color += texture2D(image, uv - (off3 / resolution)) * 0.010381362401148057;
  return color;
}

void main()
{	
  //if(u_is_horizontal)
 //    gl_FragColor = blur13(baseTexture, texcoord, vec2(u_screen_width, u_screen_height), vec2(1,0) );
// else

   //vec2 uv = vec2(gl_FragCoord.xy / vec2(u_screen_width, u_screen_height));
   //uv.y = 1.0 - uv.y;
   gl_FragColor = blur13(baseTexture, texcoord, vec2(u_screen_width, u_screen_height),  u_dir);
   //gl_FragColor = texture2D(baseTexture, texcoord);

  /*   vec4 texColor = texture( baseTexture, texcoord);
 
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
    
    gl_FragColor =  vec4(result, texColor.a);  */
}


