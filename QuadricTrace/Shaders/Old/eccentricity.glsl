#version 450

#define EPSILON 0.001
#define PI 3.14159265359

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

restrict writeonly uniform image3D ecc;
uniform sampler3D sdf_values;

uniform int useConeTrace = 0;

// cube 
uniform int ray_count = 6;
uniform vec3 ray_directions[]= { 
	vec3(0, 0, 1), 
	vec3(0, 0, -1), 
	vec3(0, 1, 0), 
	vec3(0, -1, 0), 
	vec3(1, 0, 0), 
	vec3(-1, 0, 0), 
}; 
uniform float ray_half_angles_tan[] = { 
	sqrt(2), sqrt(2), 
	sqrt(2), sqrt(2), 
	sqrt(2), sqrt(2) 
};


uniform int N = 70;
uniform int M = 70;
uniform float correction = 0.0;

TraceResult sphere_trace(Ray ray, SphereTraceDesc params, ivec3 dim)
{
	TraceResult ret = TraceResult(ray.Tmin, 0);
	float d;

	int i = 0; do
	{
		d = texelFetch(sdf_values, globalToTexel(ray.P + ret.T * ray.V,dim), 0).r;
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

/*
	Returns the direction from the center of the cell towards the surface
*/
vec3 gradient(vec3 coords)
{
	float[8] vert;
	float[6] c;
	for (int x = 0; x < 2; ++x)														//		   3-------- 7
	{																				//		  /|        /|
		for (int y = 0; y < 2; ++y)													//		 / |       / |
		{																			//		2-------- 6  |			^   /
			for (int z = 0; z < 2; ++z)												//		|  |      |  |			|y /z
			{																		//		|  1------|--5			| /
				vert[x * 4 + y * 2 + z] = 											//		| /       | /           |/    x     
					texelFetch(sdf_values, ivec3(coords + vec3(x, y, z)), 0).r;		//		|/        |/			------->
			}																		//		0-------- 4
		}
	}

	c[0] = (vert[0] + vert[1] + vert[2] + vert[3]);
	c[1] = (vert[4] + vert[5] + vert[6] + vert[7]);
	c[2] = (vert[0] + vert[1] + vert[4] + vert[5]);
	c[3] = (vert[6] + vert[7] + vert[2] + vert[3]);
	c[4] = (vert[0] + vert[4] + vert[2] + vert[6]);
	c[5] = (vert[5] + vert[1] + vert[7] + vert[3]);


	vec3 norm = vec3(c[0] - c[1], c[2] - c[3], c[4] - c[5]);

	//if close to zero, return [0,1,0]
	return length(norm) > EPSILON ? normalize(norm) : vec3(0, 1, 0);
}

struct ConeTraceDesc
{
	float epsilon;
	int maxiters;
	float startingRadius;
	float tanHalfAngle;
};

vec3 computeGradient(vec3 p) 
{
    return normalize(vec3(
        SDF(vec3(p.x + EPSILON, p.y, p.z)) - SDF(vec3(p.x - EPSILON, p.y, p.z)),
        SDF(vec3(p.x, p.y + EPSILON, p.z)) - SDF(vec3(p.x, p.y - EPSILON, p.z)),
        SDF(vec3(p.x, p.y, p.z  + EPSILON)) - SDF(vec3(p.x, p.y, p.z - EPSILON))
    ));
}

// based on tetrahedron
vec3 computeGradient2(in vec3 p ) // for function f(p)
{
    const vec2 k = vec2(1,-1);
    return normalize( k.xyy * SDF(p + k.xyy * EPSILON) + 
                      k.yyx * SDF(p + k.yyx * EPSILON) + 
                      k.yxy * SDF(p + k.yxy * EPSILON) + 
                      k.xxx * SDF(p + k.xxx * EPSILON));
}

struct ConeTraceResult
{
	float T;		// Distance taken on ray
	vec3 p;
	int flags;		// bit 0:   distance condition:     true if travelled to far t > t_max
					// bit 1:   surface condition:      true if distance to surface is small < error threshold
};                  // bit 2:   iteration condition:    true if took too many iterations

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


void main()
{
	ivec3 coords = ivec3(gl_GlobalInvocationID.xyz);
	
	vec3 norm = gradient(coords);
	vec3 p = texelToGlobal(coords, ivec3(gl_NumWorkGroups.xyz));

	float k = 1;

	if (texelFetch(sdf_values, globalToTexel(p, ivec3(gl_NumWorkGroups.xyz) + ivec3(1)), 0).r <= 0) {
		imageStore(ecc, coords, vec4(-3));
		return;
	}

	if (useConeTrace == 0) 
	{
		for (int i = 0; i < N; ++i)
		{
			for (int j = 0; j <= M; ++j)
			{
				// N*M rays using spherical coordinates
				float u = 2 * PI * i / float(N);
				float v = PI * j / float(M);
				vec3 d = normalize(vec3(cos(u) * sin(v), cos(v), sin(u) * sin(v)));
				Ray r = Ray(p,
					0.0,
					d,
					100);
				TraceResult res = sphere_trace(r, SphereTraceDesc(1, 100), ivec3(gl_NumWorkGroups.xyz) + ivec3(1));
				if (bool(res.flags & 1) || bool(res.flags & 4)) { continue; }

				// if hits the surface, evaluate minimal k
				float scale = res.T;
				float cosPhi = dot(d, norm);
				float sinPhi = length(cross(d, norm));
				vec2 pos2d = vec2(sinPhi, cosPhi) * scale;
				k = min(k, mix(getK(pos2d), -1.0, correction));
			}
		}
	} 
	else 
	{
		for (int i = 0; i < ray_count; i++)
		{
			Ray r = Ray(p,
				0.0,
				normalize(ray_directions[i]),
				100);

			ConeTraceResult res = cone_trace(r, 
				ConeTraceDesc(0.01f, 200, 0.0f, ray_half_angles_tan[i]), 
				ivec3(gl_NumWorkGroups.xyz) + ivec3(1)
			);
			if (bool(res.flags & 1) || bool(res.flags & 4)) { continue; }

			vec3 ray_dir = normalize(res.p - p);

			// if hits the surface, evaluate minimal k
			float scale = length(res.p - p);
			float cosPhi = dot(ray_dir, norm);
			float sinPhi = length(cross(ray_dir, norm));
			vec2 pos2d = vec2(sinPhi, cosPhi) * scale;
			k = min(k, getK(pos2d));
		}
		k = -k;
	}

	imageStore(ecc, coords, vec4(k, norm));
}
