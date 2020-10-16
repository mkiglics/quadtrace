#version 450

#define EPSILON 0.01
#define PI 3.14159265359

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

restrict writeonly uniform image3D ecc;
uniform sampler3D sdf_values;

uniform int useConeTrace = 0;
uniform int ray_count = 4; 
 
/* Problems with looking down at things 
*/ 
const float TETRAHEDRON_TAN = 2*sqrt(2); 
// tetrahedron 
uniform vec3 ray_directions[]= { 
	vec3(1, 1, -1) / sqrt(3), 
	vec3(-1, -1, -1) / sqrt(3), 
	vec3(1, -1, 1) / sqrt(3), 
	vec3(-1, 1, 1) / sqrt(3) 
}; 
uniform float ray_half_angles_tan[] = { 
	TETRAHEDRON_TAN, TETRAHEDRON_TAN, 
	TETRAHEDRON_TAN, TETRAHEDRON_TAN 
}; 
 
// cube 
/*uniform vec3 ray_directions[]= { 
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
};*/ 
 
// octahedron 
/*uniform vec3 ray_directions[]= { 
	vec3(1, 1, 1) / sqrt(3), 
	vec3(-1, -1, -1) / sqrt(3), 
	vec3(-1, 1, 1) / sqrt(3), 
	vec3(1, -1, 1) / sqrt(3), 
	vec3(1, 1, -1) / sqrt(3), 
	vec3(-1, -1, 1) / sqrt(3), 
	vec3(1, -1, -1) / sqrt(3), 
	vec3(-1, 1, -1) / sqrt(3), 
}; 
uniform float ray_half_angles_tan[] = { 
	2 / sqrt(2), 2 / sqrt(2), 2 / sqrt(2), 2 / sqrt(2), 
	2 / sqrt(2), 2 / sqrt(2), 2 / sqrt(2), 2 / sqrt(2) 
};*/ 
 
#define PHI 1.61803398874989484820458683436 
/*const float ICO_TAN = 4 / (3 + sqrt(5)); 
uniform vec3 ray_directions[]= { 
	vec3( (1 + PHI),  (1 + PHI),  (1 + PHI)), 
	vec3(-(1 + PHI),  (1 + PHI),  (1 + PHI)), 
	vec3( (1 + PHI), -(1 + PHI),  (1 + PHI)), 
	vec3( (1 + PHI),  (1 + PHI), -(1 + PHI)), 
	vec3(-(1 + PHI), -(1 + PHI),  (1 + PHI)), 
	vec3( (1 + PHI), -(1 + PHI), -(1 + PHI)), 
	vec3(-(1 + PHI),  (1 + PHI), -(1 + PHI)), 
	vec3(-(1 + PHI), -(1 + PHI), -(1 + PHI)), 
	vec3( PHI, 0, 1 +  2 * PHI), 
	vec3(-PHI, 0, 1 +  2 * PHI), 
	vec3( PHI, 0, 1 + -2 * PHI), 
	vec3(-PHI, 0, 1 + -2 * PHI), 
	vec3(0, -PHI, 1 + -2 * PHI), 
	vec3(0,  PHI, 1 + -2 * PHI), 
	vec3(0, -PHI, 1 +  2 * PHI), 
	vec3(0,  PHI, 1 +  2 * PHI), 
	vec3( PHI, 1 +  2 * PHI, 0), 
	vec3(-PHI, 1 +  2 * PHI, 0), 
	vec3( PHI, 1 + -2 * PHI, 0), 
	vec3(-PHI, 1 + -2 * PHI, 0) 
}; 
uniform float ray_half_angles_tan[] = { 
	ICO_TAN, ICO_TAN, ICO_TAN, ICO_TAN, 
	ICO_TAN, ICO_TAN, ICO_TAN, ICO_TAN, 
	ICO_TAN, ICO_TAN, ICO_TAN, ICO_TAN, 
	ICO_TAN, ICO_TAN, ICO_TAN, ICO_TAN, 
	ICO_TAN, ICO_TAN, ICO_TAN, ICO_TAN 
};*/ 


uniform int N = 70;
uniform int M = 70;
uniform float correction = 0.01;

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

struct ConeTraceResult
{
	float T;		// Distance taken on ray
	vec3 p;
	int flags;		// bit 0:   distance condition:     true if travelled to far t > t_max
					// bit 1:   surface condition:      true if distance to surface is small < error threshold
};                  // bit 2:   iteration condition:    true if took too many iterations

/*ConeTraceResult cone_trace(Ray ray, ConeTraceDesc desc, ivec3 dim) 
{
	ConeTraceResult ret = ConeTraceResult(ray.Tmin, vec3(0.0f), 0);

	// current distance to the object
    float d;
	float lipschitz = 1 + abs(desc.tanHalfAngle);
    
    int i = 0;
	while (i < desc.maxiters) {
		do
		{
			d = texelFetch(sdf_values, globalToTexel(ray.P + ret.T * ray.V,dim), 0).r;
			//d = SDF(ray.P + ret.T * ray.V);

			d = (d - desc.startingRadius - ret.T * desc.tanHalfAngle) / lipschitz;
			ret.T += d;
			++i;
		} while (
			ret.T < ray.Tmax     &&       		// Stay within bound box
			d	  > desc.epsilon &&	            // Stop if close to surface
			i     < desc.maxiters	        	// Stop if too many iterations
		);
	}

	// when one of the sides hit
	if (d <= desc.epsilon) {
		ret.p = ray.P + ret.T * ray.V;
		const int N = 8;

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

			// where the point is on the surface
			ivec3 texelCoord = globalToTexel(circleP, dim);
			vec3 surfaceP = circleP + gradient(texelCoord) * texelFetch(sdf_values, texelCoord, 0).r;

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
	}
    
	// bit 0:   distance condition:     true if travelled to far t > t_max
	// bit 1:   surface condition:      true if distance to surface is small < error threshold
    // bit 2:   iteration condition:    true if took too many iterations
    ret.flags =  int(ret.T >= ray.Tmax)
              | (int(d <= desc.epsilon)  << 1)
              | (int(i >= desc.maxiters) << 2);

	return ret;
}*/

ConeTraceResult cone_trace(Ray ray, ConeTraceDesc desc, ivec3 dim) 
{
	ConeTraceResult ret = ConeTraceResult(ray.Tmin, vec3(0.0f), 0);

	// current distance to the object
    float d;
	float lipschitz = 1 + abs(desc.tanHalfAngle);
    
    int i = 0; 
	
	do
    {
		d = texelFetch(sdf_values, globalToTexel(ray.P + ret.T * ray.V, dim), 0).r;
		// d = SDF(ray.P + ret.T * ray.V);

        d = (d - desc.startingRadius - ret.T * desc.tanHalfAngle) / lipschitz;
        ret.T += d;
        ++i;
    } while (
		ret.T < ray.Tmax     &&       		// Stay within bound box
		d	  > desc.epsilon &&	            // Stop if close to surface
		i     < desc.maxiters	        	// Stop if too many iterations
	);

	ret.p = ray.P + ret.T * ray.V;
	// when we are at the end and the cone intersected this vector can be used to determine
	// the intersection point of the base of the cone and the sdf
	/*if (d <= desc.epsilon) {
		vec3 grad = computeGradient(ret.p);
		// project grad onto plane at the end of the cone
		vec3 planeNormal = ray.V;

		grad = normalize(grad - dot(grad, planeNormal) * planeNormal);
		ret.p += (desc.startingRadius + ret.T * desc.tanHalfAngle + d) * grad;
	}*/
    
	// bit 0:   distance condition:     true if travelled to far t > t_max
	// bit 1:   surface condition:      true if distance to surface is small < error threshold
    // bit 2:   iteration condition:    true if took too many iterations
    ret.flags =  int(ret.T >= ray.Tmax)
              | (int(d <= desc.epsilon)  << 1)
              | (int(i >= desc.maxiters) << 2);

	return ret;
}


void main()
{
	ivec3 coords = ivec3(gl_GlobalInvocationID.xyz);

	
	vec3 norm = gradient(coords);
	vec3 p = texelToGlobal(coords,ivec3(gl_NumWorkGroups.xyz));

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
			ConeTraceResult res = cone_trace(r, ConeTraceDesc(0.1f, 100, 0.0f, ray_half_angles_tan[i]), ivec3(gl_NumWorkGroups.xyz)+ivec3(1));
			if (bool(res.flags & 1) || bool(res.flags & 4)) { continue; }

			vec3 ray_dir = normalize(res.p - p);

			// if hits the surface, evaluate minimal k
			float scale = length(ray_dir);
			float cosPhi = dot(ray_dir, norm);
			float sinPhi = length(cross(ray_dir, norm));
			vec2 pos2d = vec2(sinPhi, cosPhi) * scale;
			k = min(k, mix(getK(pos2d), -1.0, correction));
		}
	}

	imageStore(ecc, coords, vec4(k, norm));
}
