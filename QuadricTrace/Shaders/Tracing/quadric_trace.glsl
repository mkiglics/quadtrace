//?#version 450
//?#include "../SDF/SDFprimitives.glsl"
//?#include "../SDF/SDFcommon.glsl"
//?#include "../Math/interface.glsl"
//?#include "../Math/quadric.glsl"
//?#include "../Math/common.glsl"
//?#include "enhanced_sphere_trace.glsl"

TraceResult quadricTrace01(in Ray ray, in SphereTraceDesc params)
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
		c = globalToVoxel(p, N-ivec3(1));
		//direction of the surface
		
//		QuadricField quadric = loadQuadricField(inField, p);
//		k = quadric.k;
//		float dist = quadric.dist;
//		dir = quadric.normal;

		vec4 tex = texelFetch(eccentricity, c, 0);
		float dist = texelFetch(sdf_values, globalToVoxel(p, N), 0).r;
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

	return ret;

}

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

TraceResult quadricTrace(in Ray ray, in SphereTraceDesc params)
{
    TraceResult a = quadricTrace01(ray, params);
	ray.Tmin = a.T;
	TraceResult b = enhancedSphereTrace(ray, params);
	return b;
}
