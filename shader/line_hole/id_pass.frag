#version 430  compatibility

layout(location = 0) out int idOut;

uniform isampler2D idTexture;
in vec2 texcoord;

void main()
{
	vec2 delta = vec2(1) / textureSize(idTexture, 0);
	
	vec2 uv0 = texcoord;
	vec2 uv1 = texcoord + vec2(delta.x, 0);
	vec2 uv2 = texcoord + vec2(0, delta.y);
	vec2 uv3 = texcoord + vec2(delta.x, delta.y);
	
	int val0 = texture(idTexture, uv0).r;
	int val1 = texture(idTexture, uv1).r;
	int val2 = texture(idTexture, uv2).r;
	int val3 = texture(idTexture, uv3).r;
	
	if(val0 == val1 && val1 == val2 && val2 == val3)
	{
	   idOut = val0; 
	}
	else
	{ 
	  ivec4 arr = ivec4(val0, val1, val2, val3);
	    
	  //对arr排序
	  for(int i = 0; i < 4; i++)
	  {
		for(int j = i + 1; j < 4; j++)
		{
			if(arr[j] < arr[i])
			{
			   int temp = arr[i];
			   arr[i] = arr[j];
			   arr[j] = temp;
			}
		}
	  }
	  
	  int pre = 0;
	  
	  for(int i = 0; i < 4; i++)
	  {
		if(pre == 0)
		{
		   pre = arr[i];
		   continue;
		}
		
		if(pre != arr[i])
		{
		   idOut = 1; 
		   return;    //当前4个像素，有不同的构件
		}
	  }
	  
	  idOut = pre;
	}
}
