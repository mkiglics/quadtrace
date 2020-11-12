//?#version 450
//?#include "../SDF/SDFprimitives.glsl"
//?#include "../SDF/SDFcommon.glsl"

#define TRACE_OR 1.6

TraceResult relaxedSphereTracing(in Ray ray, in SphereTraceDesc params)
{
	TraceResult ret = TraceResult(ray.Tmin, 0);
	float rc = 0, rn;
	float di = 0;
	int i = 0;
	do
	{
		di = rc * TRACE_OR;
		rn = SDF(ray.P+ray.V*(ret.T + di)); 
		if(di > rc + rn)
		{
			di = rc;
			rn = SDF(ray.P+ray.V*(ret.T + di));
		}
		ret.T += di;
		rc = rn;
		++i;
	} while (
		ret.T < ray.Tmax &&       					// Stay within bound box
		rn	  > params.epsilon * (ret.T + di) &&	// Stop if cone is close to surface
		i     < params.maxiters);
	ret.flags =  int(ret.T >= ray.Tmax)
              | (int(di <= params.epsilon * (ret.T + di))  << 1)
              | (int(i >= params.maxiters) << 2);
	return ret;
}