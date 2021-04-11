#version 430 compatibility
uniform sampler2D baseTexture;
uniform sampler2D depthTexture;
uniform isampler2D idTexture;
uniform sampler2D linePtTexture;
uniform isamplerBuffer textureBuffer1;
uniform isamplerBuffer textureBuffer2;

uniform float u_out_range;
uniform float u_inner_range;

uniform bool u_line_hole_enable;
uniform bool u_always_dont_connected; //连接关系  测试用
uniform bool u_always_intersection; //判断连接时，永远相交， 测试用
uniform int u_rang_i;

in vec2 texcoord;
out vec4 fragColor;

vec4 baseColor;


#define ID_ELEMENT_DONT_PARTICIPATION 1


//与遍历id1 相连接的所以对象，查看id2是否在其中
bool is_connected(int id1, int id2)
{
	if(u_always_dont_connected)
		return false;

	//id 从1开始，零被认为是无效数据
	if(id1 == 0 || id2 == 0) return false;

	int index = texelFetch(textureBuffer1, id1).r;

	if(index == 0)
		return false;

	while(true)
	{
		int index2 = texelFetch(textureBuffer2, index).r;
		if(index2 == id2) return true;
		else if(index2 == 0) return false;  //index2 == 0, 代表所有与id1连接的对象已经遍历
		index++;
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
	float val1 = (q.y - p.y) * (r.x - q.x);

	float val2 = (q.x - p.x) * (r.y - q.y);


	// if (all(equal(vec2(val1 - val2, val2 - val1), vec2(0))))
	// return 0;  // colinear

	if(abs(val1 - val2) < 0.001) return 0;

	return (val1 - val2 > 0)? 1: 2; // clock or counterclock wise
}

// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool doIntersect(vec2 p1, vec2 q1, vec2 p2, vec2 q2)
{
	if(u_always_intersection)
		return true;
	// Find the four orientations needed for general and
	// special cases
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	// General case
	if (o1 != o2 && o3 != o4)
	{
		if(o1 != 0 && o2 != 0 && o3 != 0 && o4 != 0) //点在线上不认为相交
			return true;
	}

	if(o1 == 0 && o2 == 0 && o3 == 0 && o4 == 0)  //点在线上不认为相交, 除非是重合
	{
		int count = 0;
		// Special Cases
		// p1, q1 and p2 are colinear and p2 lies on segment p1q1
		if (o1 == 0 && onSegment(p1, p2, q1)) count++;

		// p1, q1 and q2 are colinear and q2 lies on segment p1q1
		if (o2 == 0 && onSegment(p1, q2, q1)) count++;

		// p2, q2 and p1 are colinear and p1 lies on segment p2q2
		if (o3 == 0 && onSegment(p2, p1, q2)) count++;

		// p2, q2 and q1 are colinear and q1 lies on segment p2q2
		if (o4 == 0 && onSegment(p2, q1, q2)) count++;

		if(count >= 2) return true;
	}

	return false; // Doesn't fall in any of the above cases
}

float unpackRgbaToFloat(vec4 _rgba)
{
	const vec4 shift = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
	return dot(_rgba, shift);
}

vec4 packFloatToRgba(float _value)
{
	const vec4 shift = vec4(256 * 256 * 256, 256 * 256, 256, 1.0);
	const vec4 mask = vec4(0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
	vec4 comp = fract(_value * shift);
	comp -= comp.xxyz * mask;
	return comp;
}

bool range_search_this_uv(vec2 uv, float range, int id, vec2 text_size, float depth)
{
	// if(uv.x > 1 || uv.x < 0 || uv.y > 1 || uv.y < 0)
	// return false;

	int id2 = texture(idTexture, uv).r;
	
	//不参与，不能打断别人
	if(id2 == ID_ELEMENT_DONT_PARTICIPATION) return false;

	if(id2 > 0 && id != id2 && !is_connected(id2, id)) //连接判断
	{
		float val = unpackRgbaToFloat(texture(depthTexture, uv));
		if(depth > val)  //深度比周围大，可能需要打断
		{
			//vec4 p2 = texture(linePtTexture, uv);
			//线线相交
			//bool b1 = p1 != vec4(0) && p2 != vec4(0) && doIntersect(p1.xy, p1.zw, p2.xy, p2.zw);
			//if(b1)

			//限制在圆形区域内
			// if(range > 20 && (xx * xx + yy * yy) <= 120 * 120)
			{
				baseColor = vec4(0, 0, 0, 1); //面的情况， 如果透明度为0， 则不能遮挡背景图
				return true;
			}
		}
	}

	return false;
}

//texcoord的range范围内做查询，看看是不是需要打断
//todo: 限制在圆形区域内
//todo: 超出屏幕空间的不计算
bool range_search(float range, int id, vec2 text_size, float depth)
{
	if(range <= 20 || u_rang_i == 20)
	{
		if(range > 20) range = 20;

		while(range > 0)
		{
			for(float i = -range; i <= range; i+= 1)
			{
				//up line    这种水平  竖直交替查找，不知道能不能快点
				vec2 uv = texcoord + vec2(i / text_size.x, range / text_size.y);
				if(range_search_this_uv(uv, range, id, text_size, depth)) return true;

				//left line
				vec2 uv3 = texcoord + vec2((-range) / text_size.x, i / text_size.y);
				if(range_search_this_uv(uv3, range, id, text_size, depth)) return true;

				//bottom line
				vec2 uv2 = texcoord + vec2( i / text_size.x, -range / text_size.y );
				if( range_search_this_uv(uv2, range, id, text_size, depth )) return true;
				//right line
				vec2 uv4 = texcoord + vec2(-range / text_size.x, i / text_size.y);
				if(range_search_this_uv(uv4, range, id, text_size, depth)) return true;
			}
			range -= 1;
		}
	}
	else
	{
		float range2 = float(u_rang_i);
		//只查找外轮廓， 分多次查找
		for(float i = -range2; i <= range2; i+= 1)
		{
			//up line
			vec2 uv = texcoord + vec2(i / text_size.x, range2 / text_size.y);
			if(range_search_this_uv(uv, range2, id, text_size, depth)) return true;

			//left line
			vec2 uv3 = texcoord + vec2((-range2) / text_size.x, i / text_size.y);
			if(range_search_this_uv(uv3, range2, id, text_size, depth)) return true;

			//bottom line
			vec2 uv2 = texcoord + vec2( i / text_size.x, -range2 / text_size.y );
			if( range_search_this_uv(uv2, range2, id, text_size, depth )) return true;
			//right line
			vec2 uv4 = texcoord + vec2(-range2 / text_size.x, i / text_size.y);
			if(range_search_this_uv(uv4, range2, id, text_size, depth)) return true;
		}
	}

	return false;
}

bool perpendicular_search(const vec4 lineOrig, const vec2 uv, const int idOrig, const vec2 text_size, const float depth)
{
	vec4 p1 = lineOrig;
	int id2 = texture(idTexture, uv).r;
	
	//不参与，不能打断别人
	if(id2 == ID_ELEMENT_DONT_PARTICIPATION) return false;
	
	if(id2 > 0 && idOrig != id2 && !is_connected(id2, idOrig)) //连接判断
	{
		float val = unpackRgbaToFloat(texture(depthTexture, uv));
		if(depth > val)  //深度比周围大，可能需要打断
		{
			vec4 p2 = texture(linePtTexture, uv);

			//如果是三角面
			if(p2 == vec4(0))
			{
				baseColor = vec4(0);
				return true;
			}

			//线线相交
			bool b1 = (p1 != vec4(0)) && (p2 != vec4(0)) && doIntersect(p1.xy, p1.zw, p2.xy, p2.zw);
			if(b1)
			{
				baseColor = vec4(1);
				return true;
			}
		}
	}

	return false;
}

bool do_line_break(float range, int id, vec2 text_size, float depth)
{
	vec4 lineOrig = texture(linePtTexture, texcoord);
	//如果是三角面
	if(lineOrig == vec4(0))
		return range_search(range, id, text_size, depth);

	vec2 dir = normalize(lineOrig.xy - lineOrig.zw);

	//沿着直线两侧查找
	//如果是这种方式查找，那么判断相交是不是多余的？
	//沿着直线查找，现在还没考虑线宽。只是左右两侧偏移一个像素, 如果比较宽的线且不是以三角面绘制的，可能会有问题。比如说10个像素宽度的线
	for(float i = 1; i <= range; i+= 1)
	{
		vec2 uv = texcoord + vec2(dir.x/ text_size.x, dir.y/ text_size.y) * i;
		vec2 uv1 = uv + vec2(-dir.y/ text_size.x, dir.x/ text_size.y);
		vec2 uv2 = uv + vec2(dir.y/ text_size.x, -dir.x/ text_size.y);

		vec2 uv_ = texcoord - vec2(dir.x/ text_size.x, dir.y/ text_size.y) * i;
		vec2 uv3 = uv_ + vec2(-dir.y/ text_size.x, dir.x/ text_size.y);
		vec2 uv4 = uv_ + vec2(dir.y/ text_size.x, -dir.x/ text_size.y);

		if(perpendicular_search(lineOrig, uv1,  id, text_size, depth))
			return true;

		if(perpendicular_search(lineOrig, uv2,  id, text_size, depth))
			return true;

		if(perpendicular_search(lineOrig, uv3,  id, text_size, depth))
			return true;

		if(perpendicular_search(lineOrig, uv4,  id, text_size, depth))
			return true;
	}
	return false;
}

bool is_need_break_dot_line(float range, int id, vec2 text_size, float depth)
{
	vec4 lineOrig = texture(linePtTexture, texcoord);
	vec2 dir = normalize(lineOrig.xy - lineOrig.zw);
	//沿着直线两侧查找
	for(float i = 1; i <= range; i+= 1)
	{
		vec2 uv = texcoord + vec2(dir.x/ text_size.x, dir.y/ text_size.y) * i;
		vec2 uv1 = uv + vec2(-dir.y/ text_size.x, dir.x/ text_size.y);
		vec2 uv2 = uv + vec2(dir.y/ text_size.x, -dir.x/ text_size.y);

		vec2 uv_ = texcoord - vec2(dir.x/ text_size.x, dir.y/ text_size.y) * i;
		vec2 uv3 = uv_ + vec2(-dir.y/ text_size.x, dir.x/ text_size.y);
		vec2 uv4 = uv_ + vec2(dir.y/ text_size.x, -dir.x/ text_size.y);

		if(perpendicular_search(lineOrig, uv1,  id, text_size, depth))
			return true;

		if(perpendicular_search(lineOrig, uv2,  id, text_size, depth))
			return true;

		if(perpendicular_search(lineOrig, uv3,  id, text_size, depth))
			return true;

		if(perpendicular_search(lineOrig, uv4,  id, text_size, depth))
			return true;
	}

	return false;
}

void main()
{
	baseColor = texture(baseTexture, texcoord);

	if(!u_line_hole_enable)
	{
		fragColor = baseColor;
		return;
	}

	int id = texture(idTexture, texcoord).r;
	
	if(id == ID_ELEMENT_DONT_PARTICIPATION) //不参与，不被别人打断
	{
		fragColor = baseColor;
		return;
	}
	
	if(id == 0)   //id 无效，提前返回
	{
		fragColor = baseColor;
		//fragColor = vec4(1,1,0,1);
		return;
	}

	vec2 text_size = textureSize(depthTexture, 0);
	float depth = unpackRgbaToFloat(texture(depthTexture, texcoord));

	if(id < 0)  //隐藏线打断
	{
		//fragColor =  vec4(1, 0, 0, 1); return;
		id = -id;
		float range = int(u_inner_range);
		if(is_need_break_dot_line(range, id, text_size, depth))
		{
			fragColor =  vec4(0);
			return;
		}
	}
	else
	{
		float range = int(u_out_range);
		do_line_break(range, id, text_size, depth);
	}

	fragColor = baseColor;
	//fragColor = vec4(1);
}

