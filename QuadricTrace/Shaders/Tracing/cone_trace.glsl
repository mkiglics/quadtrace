//?#version 450

//?#include "../SDF/SDFprimitives.glsl"
//?#include "../SDF/SDFcommon.glsl"
//?#include "../Math/common.glsl"


#ifndef CONE_TRACE_ALG
#define CONE_TRACE_ALG cone_trace_gradient
#endif

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

ConeTraceResult cone_trace_simple(in ConeTraceDesc desc)
{
	ConeTraceResult ret = ConeTraceResult(vec3(0.0f), 0);
	RayCone rayCone = desc.ray;

	// current distance to the object
    float d, coneSurfaceD;
	float lipschitz = 1 + abs(desc.ray.tana);
    
    int i = 0; 
	float T = 0.0f;

	do
    {
		d = SDF(rayCone.ray.P + T * rayCone.ray.V);

        coneSurfaceD = (d - desc.ray.rad - T * desc.ray.tana) / lipschitz;
		T += coneSurfaceD;

        ++i;
    } while (
		T <   rayCone.ray.Tmax &&       		// Stay within bound box
		d > desc.epsilon &&	            // Stop if close to surface
		coneSurfaceD > desc.epsilon &&
		i < desc.maxiters	        	// Stop if too many iterations
	);
	ret.p = rayCone.ray.P + T * rayCone.ray.V;
	
	// bit 0:   distance condition:     true if travelled to far t > t_max
	// bit 1:   surface condition:      true if distance to surface is small < error threshold
    // bit 2:   iteration condition:    true if took too many iterations
    ret.flags =  int(T >= rayCone.ray.Tmax)
              | (int(d <= desc.epsilon || coneSurfaceD <= desc.epsilon)  << 1)
              | (int(i >= desc.maxiters) << 2);

	return ret;
}

ConeTraceResult cone_trace_gradient(in ConeTraceDesc desc)
{
	ConeTraceResult ret = ConeTraceResult(vec3(0.0f), 0);
	RayCone rayCone = desc.ray;
	Ray ray = rayCone.ray;

	// current distance to the object
    float d, coneSurfaceD;
	float lipschitz = 1 + abs(desc.ray.tana);
    
    int i = 0; 
	float T = 0.0f;

	do
    {
		d = SDF(rayCone.ray.P + T * rayCone.ray.V);

        coneSurfaceD = (d - desc.ray.rad - T * desc.ray.tana) / lipschitz;
		T += coneSurfaceD;

        ++i;
    } while (
		T <   rayCone.ray.Tmax &&       		// Stay within bound box
		d > desc.epsilon &&	            // Stop if close to surface
		coneSurfaceD > desc.epsilon &&
		i < desc.maxiters	        	// Stop if too many iterations
	);
	ret.p = rayCone.ray.P + T * rayCone.ray.V;
	
	// when we are at the end and the cone intersected this vector can be used to determine
	// the intersection point of the base of the cone and the sdf
	if (coneSurfaceD <= desc.epsilon) {
		vec3 grad = computeGradient2(ret.p);
		// project grad onto plane at the end of the cone
		vec3 planeNormal = rayCone.ray.V;

		grad = normalize(grad - dot(grad, planeNormal) * planeNormal);
		ret.p += (desc.ray.rad + T * desc.ray.tana + coneSurfaceD) * grad;
	}
//
//	if (coneSurfaceD <= desc.epsilon)
//	{
//		vec3 grad = computeGradient2(ret.p);
//		float l;
//		float rayGradDot = dot(ray.V, grad);
//
//		// case 1 when the gradient is "inside the cone"
//		if (rayGradDot < 0) 
//		{
//			float cosaSqrd = 1.0 / (rayCone.tana * rayCone.tana + 1);
//			float cosa = sqrt(cosaSqrd);
//			float sina = sqrt(1 - cosaSqrd);
//
//			float cosb =  -rayGradDot; // dot(-ray.V, grad);
//			float sinb = sqrt(1 - cosb * cosb);
//
//			float sing = sina * cosb + cosa * sinb;
//			l = sina / sing * T;
//		}
//		// case 2 when the gradient is "outside of the cone"
//		else
//		{
//			/*float cosa = rayGradDot;
//			float sina = sqrt(1 - cosa * cosa);
//
//			float cosb = sina;
//			l = T * rayCone.tana / cosb;*/
//			vec3 grad = computeGradient2(ret.p);
//			// project grad onto plane at the end of the cone
//			vec3 planeNormal = rayCone.ray.V;
//
//			grad = normalize(grad - dot(grad, planeNormal) * planeNormal);
//			l = desc.ray.rad + T * desc.ray.tana + coneSurfaceD;
//		}
//
//		ret.p += grad * l;
//	}
	
	// bit 0:   distance condition:     true if travelled to far t > t_max
	// bit 1:   surface condition:      true if distance to surface is small < error threshold
    // bit 2:   iteration condition:    true if took too many iterations
    ret.flags =  int(T >= rayCone.ray.Tmax)
              | (int(d <= desc.epsilon || coneSurfaceD <= desc.epsilon)  << 1)
              | (int(i >= desc.maxiters) << 2);

	return ret;
}

ConeTraceResult cone_trace_double(in ConeTraceDesc desc)
{
	ConeTraceResult ret = ConeTraceResult(vec3(0.0f), 0);
	RayCone rayCone = desc.ray;
	Ray ray = rayCone.ray;

	// current distance to the object
    float d1, coneSurfaceD1;
	float lipschitz = 1 + abs(desc.ray.tana);
    
    int i = 0; 
	float T1 = ray.Tmin;

	do
    {
		d1 = SDF(rayCone.ray.P + T1 * rayCone.ray.V);

        coneSurfaceD1 = (d1 - desc.ray.rad - T1 * desc.ray.tana) / lipschitz;
		T1 += coneSurfaceD1;

        ++i;
    } while (
		T1 <   rayCone.ray.Tmax &&       		// Stay within bound box
		d1 > desc.epsilon &&	            // Stop if close to surface
		coneSurfaceD1 > desc.epsilon &&
		i < desc.maxiters	        	// Stop if too many iterations
	);
	
	//ret.p = rayCone.ray.P + T1 * rayCone.ray.V;

	// we out boys
	if (T1 >= rayCone.ray.Tmax || i >= desc.maxiters)
	{
		// bit 0:   distance condition:     true if travelled to far t > t_max
		// bit 1:   surface condition:      true if distance to surface is small < error threshold
		// bit 2:   iteration condition:    true if took too many iterations
		ret.flags =  int(T1 >= rayCone.ray.Tmax)
				  | (int(i >= desc.maxiters) << 2);

		return ret;
	}
	

	// current distance to the object
    float d2, coneSurfaceD2;
    
    i = 0; 
	float T2 = ray.Tmax;

	do
    {
		d2 = SDF(rayCone.ray.P + T2 * rayCone.ray.V);

        coneSurfaceD1 = (d2 - desc.ray.rad - T2 * desc.ray.tana) / lipschitz;
		T2 -= coneSurfaceD2;

        ++i;
    } while (
		T2 <   rayCone.ray.Tmax &&       		// Stay within bound box
		d2 > desc.epsilon &&	            // Stop if close to surface
		coneSurfaceD2 > desc.epsilon &&
		i < desc.maxiters	        	// Stop if too many iterations
	);

	mat3 basis = getOrthonormalBasis(ray.V);
	// circle middle point
	vec3 C1 = ray.P + T1 * ray.V, C2 = ray.P + T2 * ray.V;
	float R1 = rayCone.rad + T1 * rayCone.tana, R2 = rayCone.rad + T2 * rayCone.tana;
	
	const int N = 8;
	const int M = 3;
	
	float smallestCos = inf;
	for (int i = 0; i < N; i++) 
	{
		float rad = 2 * pi / N * i;
		// calculate where the point along the base is
		vec3 P1 = C1 + R1 * (basis[1] * cos(rad) + basis[2] * sin(rad));
		vec3 P2 = C2 + R2 * (basis[1] * cos(rad) + basis[2] * sin(rad));

		vec3 dir = normalize(P2 - P1);

		float sdf1 = SDF(P1), sdf2 = SDF(P2);
		// intersection happened with surface
		if (abs(sdf1) + abs(sdf2) < distance(P1, P2)) 
		{
			vec3 F1 = P1 + sdf1 * computeGradient2(P1);
			vec3 F2 = P2 + sdf2 * computeGradient2(P2);

			// cos is smaller -> angle itself is bigger
			float cosVal1 = dot(normalize(F1 - ray.P), desc.quadricNormal);
			float cosVal2 = dot(normalize(F2 - ray.P), desc.quadricNormal);
			if (cosVal1 < smallestCos) 
			{
				smallestCos = cosVal1;
				ret.p = F1;
			}

			if (cosVal2 < smallestCos)
			{
				smallestCos = cosVal2;
				ret.p = F2;
			}
		}
		// no intersection with surface
		else
		{
			for (int m = 0; m < M; m++)
			{
				vec3 Q = P1 + float(m) / (M - 1) * dir;
				Q += SDF(Q) * computeGradient2(Q);

				float cosValue = dot(normalize(Q - ray.P), desc.quadricNormal);
				if (cosValue < smallestCos)
				{
					smallestCos = cosValue;
					ret.p = Q;
				}
			}
		}
	}

	return ret;
}

ConeTraceResult cone_trace(in ConeTraceDesc desc) 
{
	return CONE_TRACE_ALG(desc);

	RayCone rayCone = desc.ray;
	ConeTraceResult ret = ConeTraceResult(vec3(0.0f), 0);

	// current distance to the object
    float d, coneSurfaceD;
	float lipschitz = 1 + abs(desc.ray.tana);
    
    int i = 0; 
	float T = 0.0f;

	do
    {
		// d = texelFetch(sdf_values, globalToTexel(ray.P + ret.T * ray.V, dim), 0).r;
		d = SDF(rayCone.ray.P + T * rayCone.ray.V);

        coneSurfaceD = (d - desc.ray.rad - T * desc.ray.tana) / lipschitz;
		T += coneSurfaceD;

        ++i;
    } while (
		T <   rayCone.ray.Tmax &&       		// Stay within bound box
		d     > desc.epsilon &&	            // Stop if close to surface
		coneSurfaceD > desc.epsilon &&
		i < desc.maxiters	        	// Stop if too many iterations
	);

	// first it takes one step to inside of object
	ret.p = rayCone.ray.P + T * rayCone.ray.V;
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
    
	// bit 0:   distance condition:     true if travelled to far t > t_max
	// bit 1:   surface condition:      true if distance to surface is small < error threshold
    // bit 2:   iteration condition:    true if took too many iterations
    ret.flags =  int(T >= rayCone.ray.Tmax)
              | (int(d <= desc.epsilon || coneSurfaceD <= desc.epsilon)  << 1)
              | (int(i >= desc.maxiters) << 2);

	return ret;
}
