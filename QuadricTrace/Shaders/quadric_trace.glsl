#define STEP_SIZE_REDUCTION 0.95
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

		if (d<10 * params.epsilon * ret.T) {						 
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
