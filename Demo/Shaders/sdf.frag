#version 410

float sphere(vec3 p, vec3 c, float r) {
	return length(p - c) - r;
}

float box(vec3 p, vec3 b) 
{
	vec3 q = abs(p) - b;
	return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float torus( vec3 p, float R, float r )
{
  vec2 q = vec2(length(p.xz)-R,p.y);
  return length(q)-r;
}

float cylinder( vec3 p, float h, float r )
{
  vec2 d = abs(vec2(length(p.xz),p.y)) - vec2(h,r);
  return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float pyramid( vec3 p, float h)
{
  float m2 = h*h + 0.25;
    
  p.xz = abs(p.xz);
  p.xz = (p.z>p.x) ? p.zx : p.xz;
  p.xz -= 0.5;

  vec3 q = vec3( p.z, h*p.y - 0.5*p.x, h*p.x + 0.5*p.y);
   
  float s = max(-q.x,0.0);
  float t = clamp( (q.y-0.5*p.z)/(m2+0.25), 0.0, 1.0 );
    
  float a = m2*(q.x+s)*(q.x+s) + q.y*q.y;
  float b = m2*(q.x+0.5*t)*(q.x+0.5*t) + (q.y-m2*t)*(q.y-m2*t);
    
  float d2 = min(q.y,-q.x*m2-q.y*0.5) > 0.0 ? 0.0 : min(a,b);
    
  return sqrt( (d2+q.z*q.z)/m2 ) * sign(max(q.z,-p.y));
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










