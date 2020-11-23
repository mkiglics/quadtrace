//?#version 450
//?#include "../SDF/SDFprimitives.glsl"
//?#include "../SDF/SDFcommon.glsl"

TraceResult sphereTrace(in Ray ray, in SphereTraceDesc params)
{
    TraceResult ret = TraceResult(ray.Tmin, 0);
    float d;
    
    int i = 0; do
    {
        d = SDF(ray.P+ret.T*ray.V);
        ret.T+=d;
        ++i;
    } while (
		ret.T < ray.Tmax &&       			// Stay within bound box
		d	  > params.epsilon &&	        // Stop if cone is close to surface
		i     < params.maxiters	        	// Stop if too many iterations
	);
    
    ret.flags =  int(ret.T >= ray.Tmax)
              | (int(d <= params.epsilon* ret.T)  << 1)
              | (int(i >= params.maxiters) << 2);

    return ret;
}