#version 430  compatibility

layout(location = 0) out int idOut;

uniform isampler2D idTexture;
in vec2 texcoord;

void main()
{
	vec2 delta = vec2(1) / textureSize(idTexture, 0);
	
	int arr[64];
	
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			int index = i * 8 + j;
			vec2 uv = texcoord + vec2(delta.x * i, delta.y * j);
			arr[index] = texture(idTexture, uv).r;
		}
	}
	
	int pre = arr[0];
	bool flag = false;
	for(int i = 1; i < 64; i++)
	{
		if(pre != arr[i]) {
			flag = true; break; 
		}
	}
	
	if(!flag)
	{
	   idOut = arr[0];
	}
	else
	{ 
	  //对arr排序
	  for(int i = 0; i < 64; i++)
	  {
		for(int j = i + 1; j < 64; j++)
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
	  
	  for(int i = 0; i < 64; i++)
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
