//?#version 450
//?#include "common.glsl"

bool intersectBox(vec3 p, vec3 v, vec3 box, out float intersection_t)
{
    vec3 boxMin = -box / 2;
    vec3 boxMax = box / 2;
    
    float tmin = (boxMin.x - p.x) / v.x; 
    float tmax = (boxMax.x - p.x) / v.x; 
 
    if (tmin > tmax) 
	{
		float temp = tmin;
		tmin = tmax;
		tmax = temp;
	}
 
    float tymin = (boxMin.y - p.y) / v.y; 
    float tymax = (boxMax.y - p.y) / v.y; 
 
    if (tymin > tymax) 
	{
		float temp = tymin;
		tymin = tymax;
		tymax = temp;
	}
 
    if ((tmin > tymax) || (tymin > tmax)) 
    {
        return false;
    }
 
    if (tymin > tmin) 
        tmin = tymin; 
 
    if (tymax < tmax) 
        tmax = tymax; 
 
    float tzmin = (boxMin.z - p.z) / v.z; 
    float tzmax = (boxMax.z - p.z) / v.z;
 
    if (tzmin > tzmax) 
	{
		float temp = tzmin;
		tzmin = tzmax;
		tzmax = temp;
	}
 
    if ((tmin > tzmax) || (tzmin > tmax)) 
    {
        return false; 
    }
 
    if (tzmin > tmin) 
        tmin = tzmin; 
 
    if (tzmax < tmax) 
        tmax = tzmax; 
 
    
    intersection_t = tmin;
    return intersection_t >= 0;
}
