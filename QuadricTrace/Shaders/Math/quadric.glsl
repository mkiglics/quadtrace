//?#version 450
//?#include "common.glsl"

#define SGN(a) ((a) < 0 ? -1 : 1)

/* Numerically stable quadratic equation solver */
vec2 solveQuadratic(float a, float b, float c)
{
	float d = b*b-4.0*a*c;
	float t1 = (-b-SGN(b)*sqrt(d))/(2.0*a);
	float t2 = c/(a*t1);
	return d<0.0 ? vec2(-inf, inf) : vec2(min(t1,t2), max(t1,t2));
}

// evaluating A(k), B(k) and C(k) functions
vec3 quadric_GetCoeffs(float k)
{
	return vec3(k*k, 2*abs(k)-1, -k);
}

// intersect

vec2 quadric_LocalIntersect(vec3 ABC, vec3 p, vec3 v)
{
	float a = dot(ABC.xyx*v,v);
	float b = 2*dot(ABC.xyx*v,p) + ABC.z*v.y;
	float c = dot(ABC.xyx*p,p) + ABC.z*p.y;
	return solveQuadratic(a,b,c);
}

// line and quadric intersections
vec2 quadric_Intersect(vec3 p, vec3 v, vec3 p0, vec3 n0, float k)
{	
	return quadric_LocalIntersect(quadric_GetCoeffs(k), p - p0, getRotation(n0) * v);
}

float quadric_Implicit(vec3 p, vec3 p0, vec3 n0, float k)
{
	//todo
	return 0.0f;
}

// returns the quadric's parameter
float quadric_ComputeParameter(vec2 local_pos)
{
	// when x=0 getK would return inf or nan otherwise
	float x = max(abs(local_pos.x), 0.0001);
	float a = (local_pos.y > 0 ? 1 : -1) * 2 * local_pos.y * local_pos.y - local_pos.y; //which branch of quadric?
	vec2 ret = solveQuadratic(x * x,  a, -local_pos.y * local_pos.y); //sorted roots
	return local_pos.y > 0 ? ret.y : ret.x; // unbounding or bounding
}
