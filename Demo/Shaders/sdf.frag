#version 450

float sphere(vec3 p, vec3 c, float r) {
	return length(p - c) - r;
}

float box(vec3 p, vec3 b) 
{
	vec3 q = abs(p) - b;
	return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float torus( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float cylinder( vec3 p, float h, float r )
{
  vec2 d = abs(vec2(length(p.xz),p.y)) - vec2(h,r);
  return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float plane( vec3 p, vec4 n )
{
  return dot(p,n.xyz) + n.w;
}

float add(float d1, float d2) 
{
	return min(d1,d2);
}

float sub(float d1, float d2) 
{
	return max(-d1,d2);
}

float intersect(float d1, float d2)
{
	return max(d1,d2);
}

float displacement(vec3 p)
{ 
	return 0.01*sin(p.x*40)+0.01*sin(p.y*20+p.z*100);
}

float displace( in float primitive, in vec3 p )
{
    float d1 = primitive;
    float d2 = displacement(p);
    return d1+d2;
}










