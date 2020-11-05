//?#version 450

//?#include "../SDF/SDFprimitives.glsl"
//?#include "../SDF/SDFcommon.glsl"

// TODO move this function when I decide whether using it or not is necessary
mat3 getOrthonormalBasis(in vec3 v) 
{
    // frisvad method
	mat3 basis;
    basis[0] = normalize(v);
    
    if (basis[0].z < -0.9999999f)
    {
        basis[1] = vec3(0.0, -1.0, 0.0f);
        basis[2] = vec3(-1.0, 0.0, 0.0);
    }
    else 
    {
        float a = 1.0 / (1.0 + basis[0].z);
        float b = -basis[0].x * basis[0].y * a;
        
        basis[1] = vec3(1.0 - basis[0].x * basis[0].x * a, b, -basis[0].x);
        basis[2] = vec3(b, 1.0 - basis[0].y * basis[0].y * a, -basis[0].y);
    }
    
    return basis;
}

ConeTraceResult cone_trace(Ray ray, ConeTraceDesc desc, ivec3 dim) 
{
	ConeTraceResult ret = ConeTraceResult(ray.Tmin, vec3(0.0f), 0);

	// current distance to the object
    float d, coneSurfaceD;
	float lipschitz = 1 + abs(desc.tanHalfAngle);
    
    int i = 0; 
	
	float highestSmallerThan0 = -999;
	float highestSmallerThan0T = 0.0f;

	do
    {
		// d = texelFetch(sdf_values, globalToTexel(ray.P + ret.T * ray.V, dim), 0).r;
		d = SDF(ray.P + ret.T * ray.V);

        coneSurfaceD = (d - desc.startingRadius - ret.T * desc.tanHalfAngle) / lipschitz;
		/*if (coneSurfaceD < 0)
		{
			if (coneSurfaceD > highestSmallerThan0)
			{
				highestSmallerThan0 = coneSurfaceD;
				highestSmallerThan0T = ret.T;
			}

			ret.T += abs(d);
		}
		else
		{
			ret.T += coneSurfaceD;
		}*/
		ret.T += coneSurfaceD;

        ++i;
    } while (
		ret.T < ray.Tmax &&       		// Stay within bound box
		d     > desc.epsilon &&	            // Stop if close to surface
		coneSurfaceD > desc.epsilon &&
		i < desc.maxiters	        	// Stop if too many iterations
	);
	/*coneSurfaceD = highestSmallerThan0;
	ret.T = highestSmallerThan0T;
	i = 0;*/

	// first it takes one step to inside of object
	ret.p = ray.P + ret.T * ray.V;
	/*Ray newRay = Ray(ret.p + computeGradient2(ret.p) * SDF(ret.p), 0.0f, ray.V, ray.Tmax - ret.T);

	float newT = 0.0f;
	float insideD = -1, insideConeD = -1;
	for (int k = 0; k < 100 && insideConeD < 0; k++) {
		insideD = SDF(newRay.P + newT * newRay.V);
		insideConeD = (insideD - desc.startingRadius - ret.T * desc.tanHalfAngle) / lipschitz;

		ret.T += abs(insideConeD);
	}*/


	// when one of the sides hit
	/*if (coneSurfaceD <= desc.epsilon) {
		const int N = 16;

		// basis for the base of the cone
		mat3 basis = getOrthonormalBasis(ray.V);
		// middle point of base
		vec3 circleMiddle = ret.p;
		float circleMiddleLength = ret.T;
		// radius of base
		float R = desc.startingRadius + ret.T * desc.tanHalfAngle;

		// for min search
		vec3 smallestPoint;
		float smallestAngle = inf;

		for (int i = 0; i < N; i++) {
			float rad = 2 * PI / N * i;
			// calculate where the point along the base is
			vec3 circleP = circleMiddle + R * (basis[1] * cos(rad) + basis[2] * sin(rad));

			// closest point on the surface
			// ivec3 texelCoord = globalToTexel(circleP, dim);
			vec3 surfaceP = circleP + computeGradient2(circleP) * SDF(circleP);

			vec3 surfaceVec = surfaceP - ray.P;
			// calculate the cos for the surface vector and the base middle vector
			// the smaller it is, the higher the angle is between them
			float cosAngle = dot(surfaceVec, circleMiddle - ray.P) / circleMiddleLength / length(surfaceVec);

			if (cosAngle < smallestAngle) {
				smallestAngle = cosAngle;
				smallestPoint = surfaceP;
			}
		}

		ret.p = smallestPoint;
	}*/

	// when we are at the end and the cone intersected this vector can be used to determine
	// the intersection point of the base of the cone and the sdf
	if (coneSurfaceD <= desc.epsilon) {
		vec3 grad = computeGradient2(ret.p);
		// project grad onto plane at the end of the cone
		vec3 planeNormal = ray.V;

		grad = normalize(grad - dot(grad, planeNormal) * planeNormal);
		ret.p += (desc.startingRadius + ret.T * desc.tanHalfAngle) * grad;
	}

	/*if (coneSurfaceD <= desc.epsilon)
	{
		vec3 grad = computeGradient2(ret.p);
		float l;
		float rayGradDot = dot(ray.V, grad);

		// case 1 when the gradient is "inside the cone"
		if (rayGradDot < 0) 
		{
			float cosaSqrd = 1.0 / (desc.tanHalfAngle * desc.tanHalfAngle + 1);
			float cosa = sqrt(cosaSqrd);
			float sina = sqrt(1 - cosaSqrd);

			float cosb =  -rayGradDot; // dot(-ray.V, grad);
			float sinb = sqrt(1 - cosb * cosb);

			float sing = sina * cosb + cosa * sinb;
			l = sina / sing * ret.T;
		}
		// case 2 when the gradient is "outside of the cone"
		else
		{
			float cosa = rayGradDot;
			float sina = sqrt(1 - cosa * cosa);

			float cosb = sina;
			l = ret.T * desc.tanHalfAngle / cosb;
		}

		ret.p += grad * l;
	}*/
    
	// bit 0:   distance condition:     true if travelled to far t > t_max
	// bit 1:   surface condition:      true if distance to surface is small < error threshold
    // bit 2:   iteration condition:    true if took too many iterations
    ret.flags =  int(ret.T >= ray.Tmax)
              | (int(d <= desc.epsilon || coneSurfaceD <= desc.epsilon)  << 1)
              | (int(i >= desc.maxiters) << 2);

	return ret;
}
