#version 430 compatibility
uniform sampler2D baseTexture;
uniform sampler2D depthTexture;
uniform sampler2D idTexture;
uniform sampler2D linePtTexture;
uniform isamplerBuffer textureBuffer1;
uniform isamplerBuffer textureBuffer2;

in vec2 texcoord;
out vec4 fragColor;

bool is_equal(in vec4 v1, in vec4 v2)
{
	vec4 delta = abs(v1 - v2);
	if(delta.r + delta.g + delta.b + delta.a < 0.01)
		return true;

	return false;
}

int to_int(float f)
{
	return int(f);
}

//与遍历id1 相连接的所以对象，查看id2是否在其中
bool is_connected(int id1, int id2)
{
	//id 从1开始，零被认为是无效数据
	if(id1 == 0 || id2 == 0) return false;

	int index = texelFetch(textureBuffer1, id1).r;

	if(index == 0)
		return false;


	int max_connected = 100;
	int i = 0;

	while(true)
	{
		int index2 = texelFetch(textureBuffer2, index).r;
		if(index2 == id2) return true;
		else if(index2 == 0) return false;  //index2 == 0, 代表所有与id1连接的对象已经遍历
		index++;

		i++;
		if(i > max_connected) return false;
	}

	return false;
}


// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool onSegment(vec2 p, vec2 q, vec2 r)
{
	if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
	        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
		return true;

	return false;
}
//https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(vec2 p, vec2 q, vec2 r)
{
	// See https://www.geeksforgeeks.org/orientation-3-ordered-points/
	// for details of below formula.
	float val = (q.y - p.y) * (r.x - q.x) -
	            (q.x - p.x) * (r.y - q.y);

	if (abs(val) < 0.00001) return 0;  // colinear

	// if(val == 0) return 0;

	return (val > 0)? 1: 2; // clock or counterclock wise
}

// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool doIntersect(vec2 p1, vec2 q1, vec2 p2, vec2 q2)
{
	// Find the four orientations needed for general and
	// special cases
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	// General case
	if (o1 != o2 && o3 != o4)
		return true;

	// Special Cases
	// p1, q1 and p2 are colinear and p2 lies on segment p1q1
	if (o1 == 0 && onSegment(p1, p2, q1)) return true;

	// p1, q1 and q2 are colinear and q2 lies on segment p1q1
	if (o2 == 0 && onSegment(p1, q2, q1)) return true;

	// p2, q2 and p1 are colinear and p1 lies on segment p2q2
	if (o3 == 0 && onSegment(p2, p1, q2)) return true;

	// p2, q2 and q1 are colinear and q1 lies on segment p2q2
	if (o4 == 0 && onSegment(p2, q1, q2)) return true;

	return false; // Doesn't fall in any of the above cases
}


float unpackRgbaToFloat(vec4 _rgba)
{
	const vec4 shift = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
	return dot(_rgba, shift);
}

bool is_hidden_line(vec4 color)
{
	return is_equal(color, vec4(1,1,1,1));
}

bool is_out_the_face(const int id, vec2 uv, const vec2 text_size)
{
	int cnt = 0;
	int cnt2 = 0;
	for (int i = -1; i <= 1; i++)
	{
		for(int j = -1; j <= 1; j++)
		{
			vec2 uv2 = uv + vec2(i / text_size.x, j / text_size.y);

		}
	}

	return false;
}

bool is_line_face_intersection(vec2 uv, const vec2 text_size)
{
	int cnt = 0;
	int cnt2 = 0;
	for (int i = -1; i <= 1; i++)
	{
		for(int j = -1; j <= 1; j++)
		{
			vec2 uv2 = uv + vec2(i / text_size.x, j / text_size.y);
			int id2 = to_int(texture(idTexture, uv2).r);
			if(id2 == 999) return true;
		}
	}

	return false;
}

void main()
{
	const vec4 baseColor = texture(baseTexture, texcoord);
	int id = to_int(texture(idTexture, texcoord).r);
	int range = 10;

	if(id == 999)
	{
		//fragColor = vec4(1, 0, 1, 1); return;
	}

	// vec2 text_size2 = textureSize(depthTexture, 0);
	// if(text_size2.x == 1024 && text_size2.y == 1023.9999)
	// {
	// fragColor = vec4(1,1,0,1); return;
	// }
	//vec4 pt = abs(texture(linePtTexture, texcoord));
	//fragColor = pt; return;

	if(id != 0)
		//if(false)
	{
		vec2 text_size = textureSize(depthTexture, 0);
		float depth = unpackRgbaToFloat(texture(depthTexture, texcoord));
		int range2 = 25;
		vec2 delta = vec2(1.0 / text_size.x, 1.0 / text_size.y);

		if(is_hidden_line(baseColor))  //隐藏线打断
		{	
			//id = -id;
			for(int i = -range2; i <= range2; ++i)
			{
				for(int j = -range2; j <= range2; ++j)
				{
					vec2 uv = texcoord + vec2(i / text_size.x, j / text_size.y);
					int id2 = to_int(texture(idTexture, uv).r);

					if(id2 != 0 && id != id2 && !is_connected(id2, id)) //连接判断
					{
						vec4 p1 = texture(linePtTexture, uv);
						vec4 p2 = texture(linePtTexture, texcoord);
						if(!is_equal(p1, vec4(0)) && !is_equal(p2, vec4(0))
						        && doIntersect(p1.xy, p1.zw, p2.xy, p2.zw))
						{
							fragColor =  vec4(1,0,0,1);
							return;
						}
					}
				}
			}
		}
		else
		{
			for(int i = -range; i <= range; ++i)
			{
				for(int j = -range; j <= range; ++j)
				{
					vec2 uv = texcoord + vec2(i / text_size.x, j / text_size.y);
					int id2 = to_int(texture(idTexture, uv).r);
					if(id2 != 0 && id != id2 && !is_connected(id2, id)) //连接判断
					{
						float val = unpackRgbaToFloat(texture(depthTexture, uv));
						if(val < depth)  //深度比周围大，可能需要打断
						{
							vec4 p1 = texture(linePtTexture, texcoord);
							vec4 p2 = texture(linePtTexture, uv);
							//线线相交
							bool b1 = !is_equal(p1, vec4(0)) && !is_equal(p2, vec4(0)) && doIntersect(p1.xy, p1.zw, p2.xy, p2.zw);
							if(b1)
							{
								fragColor =  vec4(0,1,1,1);
								return;
							}
						}
					}
				}
			}
		}
	}

	// vec4 result2 = texture(linePtTexture, texcoord);
//  fragColor = result2;
	// return;

	//vec4 dp = texture(depthTexture, texcoord);

	fragColor = baseColor;
}
// 线把面打断的情况--------------------

































