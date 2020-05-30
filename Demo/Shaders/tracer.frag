#version 450

#define SGN(a) (a < 0 ? -1 : 1)
//#define S 1.0

//texture to global coordinates
vec3 l2g(ivec3 p, ivec3 n)
{
	return (p-(n-vec3(1))/2.0);
}

//global to texture coordinates
ivec3 g2l(vec3 p, ivec3 n)
{
	return clamp(ivec3(round(p + (n-vec3(1))/2.0)), ivec3(0), ivec3(n-ivec3(1)));
}

struct Ray
{
	vec3 P;
	float Tmin;
	vec3 V;
	float Tmax;
};

struct TraceResult
{
	float T;		// Distance taken on ray
	int flags;		// bit 0:   distance condition:     true if travelled to far t > t_max
					// bit 1:   surface condition:      true if distance to surface is small < error threshold
};                  // bit 2:   iteration condition:    true if took too many iterations

struct SphereTraceDesc
{
	float epsilon;  //Stopping distance to surface
	int maxiters;   //Maximum iteration count
};

Ray Camera(vec2 pix, vec3 eye, vec3 at, vec2 dim)
{    
	vec3 w = normalize(at-eye);
    vec3 u = normalize(cross(w,vec3(0,1,0)));
	vec3 v = cross(u,w);
    
    vec2 px = (pix*2.-1.)*1.*normalize(dim);
    
    return Ray(eye,							//P
               0.1,							//minT
               normalize(w+px.x*u+px.y*v),	//V
               100.);						//maxT
}

mat3 getRotation(vec3 normal)
{
	vec3 y = vec3(0, 1, 0);
	vec3 v = cross(normal, y);
	float c = dot(normal, y);
	if (c < -0.99) {
		return mat3(1, 0, 0, 0, -1, 0, 0, 0, -1);
	}
	mat3 m = mat3(0, v.z, -v.y, -v.z, 0, v.x, v.y, -v.x, 0);
	return mat3(1) + m + m * m / (1 + c);
}

const float inf = 1. / 0.; // at least OpenGL 4.1

vec2 solveQuadratic(float a, float b, float c)
{
	float d = b*b-4*a*c;
	float t1 = (-b-SGN(b)*sqrt(d))/(2*a);
	float t2 = c/(a*t1);
	return d<0 ? vec2(-inf, inf) : vec2(min(t1,t2), max(t1,t2));
}

float getGeogebraK(vec2 pos)
{	
	//g = 0
//	if (pos.y > 0)
//		return pos.y*pos.y/( pos.x*pos.x+2*pos.y*pos.y-pos.y);
//	return pos.y*pos.y/(-pos.x*pos.x-2*pos.y*pos.y-pos.y);

	//g = -1
	return pos.y>0 ? solveQuadratic(pos.x*pos.x, 2*pos.y*pos.y-pos.y, -pos.y*pos.y).y : solveQuadratic(pos.x*pos.x, -2*pos.y*pos.y-pos.y, -pos.y*pos.y).x;
}
