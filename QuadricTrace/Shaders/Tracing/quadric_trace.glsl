//?#version 450

//?#include "../Math/common.glsl"
//?#include "../SDF/SDFprimitives.glsl"
//?#include "../SDF/SDFcommon.glsl"
//?#include "../Math/interface.glsl"
//?#include "../Math/quadric.glsl"
//?#include "enhanced_sphere_trace.glsl"


TraceResult trace(in Ray ray, in SphereTraceDesc params, restrict in readonly image3D inField)
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
//		c =  globalToTexel(p, N-ivec3(1));
		//direction of the surface
		QuadricField quadric = loadQuadricField(inField, p);
		k = quadric.k;
		float dist = quadric.dist;
		dir = quadric.normal;
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

		//the parameter of the closest intersection point
		ivec3 quadricPos = globalToVoxel(p, imageSize(inField));
		t = quadric_IntersectClosest(p, ray.V, voxelToGlobal(quadricPos, imageSize(inField)), dir, k);
		d = t;//max(t, dist - 1);
	
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

/*
TraceResult quadricTraceField(in Ray ray, in SphereTraceDesc params, restrict in readonly image3D inField)
{
	TraceResult ret = TraceResult(ray.Tmin, 0);
	vec3 p, dir;
	float k, t, d;
	mat3 rot;
	int i = 0; do
    {
		p = ray.P+ret.T*ray.V;
		QuadricField field = decodeQuadricField(imageLoad(inField, globalToVoxel(p, N-ivec3(1))));


		float dist = field.dist;
		dir = field.normal;
		k = field.k;
		rot = getRotation(dir);
		t = quadric_Intersect_Closest(p, ray.V, round(p), dir, k);
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
*/

TraceResult quadricTrace(Ray ray, in SphereTraceDesc params, layout(rgba32f) restrict readonly image3D inField)
{
    TraceResult a = quadricTrace01(ray, params, inField);
	return a;
	ray.Tmin = a.T;
	TraceResult b = enhancedSphereTrace(ray, params);
//	b = trace(ray, params, inField);
	return b;
}
