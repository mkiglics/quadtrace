//?#version 450

//?#include "../SDF/SDFcommon.glsl"
//?#include "../SDF/SDFprimitives.glsl"
//?#include "../Tracing/cone_trace.glsl"
//?#include "../Tracing/enhanced_sphere_trace.glsl"
//?#include "../Math/quadric.glsl"
//?#include "../Math/interface.glsl"
//?#include "constants.glsl"

// Based on exact SDF

#define EPSILON 0.001
#define PI 3.14159265359

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

restrict writeonly uniform image3D outField;

#ifndef UNBOUND_QUADRIC
#define UNBOUND_QUADRIC unboundQuadricBruteForce
#endif

uniform ivec2 uSampleResolution = ivec2(10,10);

/* The brute force method (sending rays in a lot of directions) for computing k.
*/
float unboundQuadricBruteForce(vec3 p, vec3 n, float dist)
{
	float k = -1;

	for (int i = 0; i < uSampleResolution.x; ++i)
	{
		for (int j = 0; j <= uSampleResolution.y; ++j)
		{
			// N*M rays using spherical coordinates
			float u = 2 * PI * i / float(uSampleResolution.x);
			float v = PI * j / float(uSampleResolution.y);
			vec3 d = normalize(vec3(cos(u) * sin(v), cos(v), sin(u) * sin(v)));
			Ray r = Ray(p, 0.0, d, 100);

			TraceResult res = enhancedSphereTrace(r, SphereTraceDesc(1, 100));
			if (bool(res.flags & 1) || bool(res.flags & 4)) { continue; }

			// if hits the surface, evaluate minimal k
			float scale = res.T;
			float cosPhi = dot(d, n);
			float sinPhi = length(cross(d, n));
			vec2 pos2d = vec2(sinPhi, cosPhi) * scale;
			k = min(k, mix(quadric_ComputeParameter(pos2d), -1.0, correction));
		}
	}

	return k;
}

float unboundQuadricConeTrace(vec3 p, vec3 n, float dist)
{
	float k = -1;

	int ray_count = RAY_DIRECTIONS.length();
	for (int i = 0; i < ray_count; i++)
	{
		Ray r = Ray(p,
			0.0,
			normalize(RAY_DIRECTIONS[i]),
			100);

		ConeTraceResult res = cone_trace(r, 
			ConeTraceDesc(0.01f, 200, 0.0f, RAY_HALF_TANGENTS[i]), 
			ivec3(gl_NumWorkGroups.xyz) + ivec3(1)
		);
		if (bool(res.flags & 1) || bool(res.flags & 4)) { continue; }

		vec3 ray_dir = normalize(res.p - p);

		// if hits the surface, evaluate minimal k
		float scale = length(res.p - p);
		float cosPhi = dot(ray_dir, n);
		float sinPhi = length(cross(ray_dir, n));
		vec2 pos2d = vec2(sinPhi, cosPhi) * scale;
		k = min(k, quadric_ComputeParameter(pos2d));
	}
	k = -k;

	return k;
}

void main()
{
	ivec3 coords = ivec3(gl_GlobalInvocationID.xyz);
	vec3 p = voxelToGlobal(coords, ivec3(gl_NumWorkGroups.xyz));

	QuadricField ret;
	ret.dist = SDF(p);
	ret.normal = computeGradient(p);
	ret.k = UNBOUND_QUADRIC(p, ret.normal, ret.dist);

	storeQuadricField(outField, coords, ret);
}
