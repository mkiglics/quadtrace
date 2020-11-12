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

/*
	Returns a rotation matrix for rotating normal to (0, length(normal), 0)
	using Rodrigues' formula
*/
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

// evaluating A(k), B(k) and C(k) functions
vec3 quadric_GetCoeffs(float k)
{
	return vec3(k*k, 2*abs(k)-1, -k);
}

// intersect

// line and quadric at origin intersection
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
	mat3 rotation = getRotation(n0);
	return quadric_LocalIntersect(quadric_GetCoeffs(k), rotation * (p - p0), rotation * v);
}

// ray and any quadric intersection that returns only the first intersection point
// along the ray
float quadric_IntersectClosest(vec3 p, vec3 v, vec3 p0, vec3 n0, float k)
{
	vec2 t12 = quadric_Intersect(p, v, p0, n0, k);
	float t1 = min(t12.x,t12.y), t2 = max(t12.x,t12.y);
	
	if( (p.y+t1*v.y)*k < 0 ) t1 = -inf;
	if( (p.y+t2*v.y)*k < 0 ) t2 = inf;

	float t = 0;
	if (k < 0) {
		if (t2 == inf) t = t1;
		else if (t1<0 && t1>-inf) t = t2;
		else if (t2 > 0) t = 0;
		else t = inf;
	} else  {
		if (t1==-inf) t = t2;
		else if (t2>0 && t2<inf) t = t1;
		else if (t1<0) t = inf;
	}
	return t;
}

/* 
 * Returns the quadric's normal at the given local space point
*/
vec3 quadric_LocalGetNormal(vec3 point, float k) 
{
	vec3 ABC = quadric_GetCoeffs(k);

	// from the implicit function's gradient
	return normalize(vec3(
		2 * ABC.x * point.x,
		2 * ABC.y * point.y + ABC.z,
		2 * ABC.x * point.z
	));
}

/* 
 * Returns the quadric's normal at the given world space point
*/
vec3 quadric_GetNormal(vec3 p, vec3 p0, vec3 n0, float k) 
{
	vec3 ABC = quadric_GetCoeffs(k);
	mat3 rot = getRotation(n0);

	// from the implicit function's gradient
	return rot * quadric_LocalGetNormal(rot * (p - p0), k);
}

float quadric_Intersect_Closest(vec3 p, vec3 v, vec3 p0, vec3 n0, float k)
{
	vec2 t12 = quadric_Intersect(p, v, p0, n0, k);
	float t1 = min(t12.x,t12.y), t2 = max(t12.x,t12.y);
	
	if( (p.y+t1*v.y)*k < 0 ) t1 = -inf;
	if( (p.y+t2*v.y)*k < 0 ) t2 = inf;

	float t = 0;
	if (k < 0) {
		if (t2 == inf) t = t1;
		else if (t1<0 && t1>-inf) t = t2;
		else if (t2 > 0) t = 0;
		else t = inf;
	} else  {
		if (t1==-inf) t = t2;
		else if (t2>0 && t2<inf) t = t1;
		else if (t1<0) t = inf;
	}
	return t;
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
