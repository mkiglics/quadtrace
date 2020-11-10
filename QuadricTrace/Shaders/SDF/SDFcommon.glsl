//?#version 450

//?#include "../Math/common.glsl"
//?#include "SDFprimitives.glsl"

// #define RAY(r,t) (r.Direction*(t)+r.Origin)
// #define SDF(r,t) sdf(RAY(r,t))
// #define CONE_SDF(raycone, t) ((SDF(raycone.ray,t)-(t)*raycone.tana-raycone.rad)/(1.0+raycone.tana)) 
// #define CONE_SDF_FOS(fos, raycone, t) (((fos)-(t)*raycone.tana-raycone.rad)/(1.0+raycone.tana)) 

struct Ray
{
	vec3 P;
	float Tmin;
	vec3 V;
	float Tmax;
};

struct RayCone
{
    Ray ray;		//Central axis of the cone
	float tana;		//Cone opening angle when cone tracing can be zero
	float rad;		//Cone starting radius at Origin (t=0)
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

struct ConeTraceResult
{
	vec3 p;
	int flags;		// bit 0:   distance condition:     true if travelled to far t > t_max
					// bit 1:   surface condition:      true if distance to surface is small < error threshold
};                  // bit 2:   iteration condition:    true if took too many iterations

struct ConeTraceDesc
{
	float epsilon;
	int maxiters;
	RayCone ray;
};

float SDF(vec3 p);

//'octahedron' symmetric difference
vec3 computeGradient(vec3 p) 
{
    const vec2 e0 = vec2(0.001, 0);
    return normalize(vec3(
        SDF(p+e0.xyy)-SDF(p-e0.xyy),
        SDF(p+e0.yxy)-SDF(p-e0.yxy),
        SDF(p+e0.yyx)-SDF(p-e0.yyx)
    )); 
}

// based on tetrahedron
vec3 computeGradient2(in vec3 p )
{
    const vec2 k = vec2(1,-1);
    const float e = 0.001;
    return normalize( k.xyy * SDF(p + k.xyy * e) + 
                      k.yyx * SDF(p + k.yyx * e) + 
                      k.yxy * SDF(p + k.yxy * e) + 
                      k.xxx * SDF(p + k.xxx * e));
}
