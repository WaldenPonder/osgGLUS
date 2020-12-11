uniform sampler2D baseTexture;
uniform sampler2D depthTexture;
uniform sampler2D idTexture;

varying vec2 texcoord;

bool is_equal(in vec4 v1, in vec4 v2)
{
   vec4 delta = abs(v1 - v2);
   if(delta.r + delta.g + delta.b < 0.01)
    return true;
	
	return false;
}

void main()
{	
    vec4 result = texture( baseTexture, texcoord);
	vec4 depth = texture(depthTexture, texcoord);
	vec4 id = texture(idTexture, texcoord);
	
    float avg = 0;
	float cnt = 0;
	int range = 10;
	
    for(int i = -range; i <= range; ++i)                                                                                             
    {                                                                                                                         
        for(int j = -range; j <= range; ++j)                                                                                          
        {                   
           vec2 uv = texcoord + vec2(i / 1920.0, j / 1080.0);		
		   vec4 id2 = texture(idTexture, uv);
		   
		   if(!is_equal(id, vec4(0)) && !is_equal(id2, vec4(0))  && !is_equal(id, id2))
		   {
		      // gl_FragColor =  vec4(1,1,0,1);
			 //  return;
		      float val = texture(depthTexture, uv).r;
			  if(val < depth.r)
			  {
			    gl_FragColor =  vec4(0,0,0,1);
				return;
			  }
		   }        
        }                                                                                                                 
    }                                                                                                                       

   gl_FragColor = result;
}
