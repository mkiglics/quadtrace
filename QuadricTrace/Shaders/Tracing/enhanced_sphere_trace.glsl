//?#version 450
//?#include "../SDF/SDFprimitives.glsl"
//?#include "../SDF/SDFcommon.glsl"

#define STEP_SIZE_REDUCTION 0.95

TraceResult enhancedSphereTrace(in Ray ray, in SphereTraceDesc params)
{
	TraceResult ret = TraceResult(ray.Tmin, 0);
	float rp = 0, rc = 0, rn = 0; //prev, curr, next
	float di = 0;
	int i = 0;
	do {
		di = rc + STEP_SIZE_REDUCTION * rc * max( (di - rp + rc) / (di + rp - rc), 0.6);
		rn = SDF(ray.P+ray.V*(ret.T + di));
		if(di > rc + rn)
		{
			di = rc;
			rn = SDF(ray.P+ray.V*(ret.T + di));
		}
		ret.T += di;
		rp = rc; rc = rn;
		++i;
	} while (
		ret.T < ray.Tmax &&       			// Stay within bound box
		rn	  > params.epsilon * ret.T &&	// Stop if cone is close to surface
		i     < params.maxiters);

	ret.flags =  int(ret.T >= ray.Tmax)
              | (int(rn <= params.epsilon)  << 1)
              | (int(i >= params.maxiters) << 2); 
	return ret;
}