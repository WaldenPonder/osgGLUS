#version 330 core

out vec4 FragColor;
uniform float radius;

in vec3 pos_;
in vec3 center_;
/*
ShadeInfo Sphere::intersect(const Ray& ray)
{
	ShadeInfo info;

	Vec3 v      = ray.orig - _center;
	float a0    = v.length2() - _radius * _radius;
	float DdotV = ray.dir * v;

	if (DdotV <= 0)
	{
		float discr = DdotV * DdotV - a0;
		float t = -DdotV - sqrt(discr);

		if (discr >= 0 && t > .001)
		{
			info.setShape(this);
			info.dis = t;
			info.hit_pos = ray.distance(info.dis);
			info.normal   = info.hit_pos - _center;
			info.normal.normalize();
			info.ray      = ray;
		}
	}

	return info;
}
*/
void main()
{
	vec3 dir = normalize(pos_);

	vec3 v      = -center_;
	float len   = length(v);
	float a0    =   len * len - radius * radius;
	float DdotV = dot(dir, v);
    
	if (DdotV <= 0)
	{
		float discr = DdotV * DdotV - a0;
		float t = -DdotV - sqrt(discr);

		if (discr >= 0 && t > .0001)
		{
		  vec3 normal = t * dir - center_;
		  normal = normalize(normal);
		  float f = abs(dot(normal, dir));		  
		  FragColor = vec4(f, 0, 0, 1);
		  
		  return;
		}
	}	
	//FragColor = vec4(0, 1, 0, 1);
	discard;
}