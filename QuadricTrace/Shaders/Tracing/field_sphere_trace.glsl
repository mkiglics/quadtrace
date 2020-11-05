//?#version 450
//?#include "../SDF/SDFprimitives.glsl"
//?#include "../SDF/SDFcommon.glsl"
//?#include "../Math/interface.glsl"

TraceResult fieldSphereTrace(Ray ray, SphereTraceDesc params, layout(r32f) image3D in_field)
{
	TraceResult ret = TraceResult(ray.Tmin, 0);
	float d;

	int i = 0; do
	{
		d = loadQuadricField(in_field, ray.P + ret.T * ray.V).dist;
		ret.T += d;
		++i;
	} while (
		ret.T < ray.Tmax &&       		// Stay within bound box
		d	  > params.epsilon &&		// Stop if cone is close to surface
		i < params.maxiters	        	// Stop if too many iterations
		);

	ret.flags = int(ret.T >= ray.Tmax)
		| (int(d <= params.epsilon) << 1)
		| (int(i >= params.maxiters) << 2);
	return ret;
}