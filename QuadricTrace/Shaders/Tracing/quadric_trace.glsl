//?#version 450

//?#include "../Math/common.glsl"
//?#include "../SDF/SDFprimitives.glsl"
//?#include "../SDF/SDFcommon.glsl"
//?#include "../Math/interface.glsl"
//?#include "../Math/quadric.glsl"
//?#include "enhanced_sphere_trace.glsl"

/*
TraceResult trace(in Ray ray, in SphereTraceDesc params)
{
    TraceResult ret = TraceResult(ray.Tmin, 0);
    float d;
	vec3 dir, p;
	ivec3 c;
	float k, t;
	mat3 rot;
    
    int i = 0; do
    {
		p = ray.P+ret.T*ray.V;

		//center of the current cell
		c =  globalToTexel(p, N-ivec3(1));
		//direction of the surface
		vec4 tex = texelFetch(eccentricity, c, 0);
		float dist = texelFetch(sdf_values, globalToTexel(p, N), 0).r;
		dir = tex.yzw;
		//parameter of the quadric
		k = tex.x;
		rot = getRotation(dir);
		//the parameter of the closest intersection point
		t = intersectQuadric(rot*(p-round(p)), rot*ray.V, k);
		d = max(t, dist - 0.7);

		if (d < 10 * params.epsilon * ret.T) 
		{						 
			break;
		}
		ret.T += d;
        ++i;
    } while (
		ret.T < ray.Tmax &&       			// Stay within bound box
		d	  > params.epsilon * ret.T &&	// Stop if close to surface
		i     < params.maxiters	        	// Stop if too many iterations
	);

	float rp = 0, rc = 0, rn = inf; //prev, curr, next
	do
	{
		d = rc + STEP_SIZE_REDUCTION * rc * max( (d - rp + rc) / (d + rp - rc), 0.6);
		rn = SDF(ray.P+ray.V*(ret.T + d));
		if(d > rc + rn)
		{
			d = rc;
			rn = SDF(ray.P+ray.V*(ret.T + d));
		}
		ret.T += d;
		rp = rc; rc = rn;
		++i;
	} while (ret.T < ray.Tmax &&       			// Stay within bound box
		rn	  > params.epsilon * ret.T &&	// Stop if cone is close to surface
		i     < params.maxiters);
    
    ret.flags =  int(ret.T >= ray.Tmax)
              | (int(d <= params.epsilon* ret.T)  << 1)
              | (int(i >= params.maxiters) << 2);
    return ret;
}
*/

TraceResult quadricTrace01(in Ray ray, in SphereTraceDesc params, layout(rgba32f) restrict readonly image3D inField)
{
	TraceResult ret = TraceResult(ray.Tmin, 0);
    float d;
	vec3 dir, p;
	ivec3 c;
	float k, t;
	mat3 rot;
    
    int i = 0; do
    {
		p = ray.P+ret.T*ray.V;

		QuadricField quadric = loadQuadricField(inField, p);
		k = quadric.k;
		float dist = quadric.dist;
		dir = quadric.normal;

		rot = getRotation(dir);
		//the parameter of the closest intersection point
		//t = quadric_LocalIntersectClosest(rot*(p - round(p)), rot * ray.V, k);
		ivec3 quadricPos = globalToVoxel(p, imageSize(inField));
		t = quadric_IntersectClosest(p, ray.V, quadricPos, dir, k);
		d = max(t, dist - 0.7);

		if (d < 10 * params.epsilon * ret.T) 
		{						 
			break;
		}
		ret.T += d;
        ++i;
    } while (
		ret.T < ray.Tmax &&       			// Stay within bound box
		d	  > params.epsilon * ret.T &&	// Stop if close to surface
		i     < params.maxiters	        	// Stop if too many iterations
	);

	return ret;

}

TraceResult quadricTrace(Ray ray, in SphereTraceDesc params, layout(rgba32f) restrict readonly image3D inField)
{
    // TraceResult a = quadricTrace01(ray, params, inField);
	// ray.Tmin = a.T;
	TraceResult b = enhancedSphereTrace(ray, params);
	return b;
}
