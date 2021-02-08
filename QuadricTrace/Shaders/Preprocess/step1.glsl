//?#version 450

//?#include "../SDF/SDFcommon.glsl"
//?#include "../SDF/SDFprimitives.glsl"
//?#include "../Tracing/cone_trace.glsl"
//?#include "../Tracing/sphere_trace.glsl"
//?#include "../Tracing/enhanced_sphere_trace.glsl"
//?#include "../Math/common.glsl"
//?#include "../Math/quadric.glsl"
//?#include "../Math/interface.glsl"
//?#include "constants.glsl"

// Based on exact SDF

#define EPSILON 0.001
#define PI 3.14159265359

// defining work group size
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// image to be written in
layout(binding = 0) restrict writeonly uniform image3D outField;

#ifndef UNBOUND_QUADRIC
#define UNBOUND_QUADRIC unboundQuadricBruteForce
#endif

uniform float uCorrection = 0.01f;
uniform ivec2 uSampleResolution = ivec2(40, 40);

/* The brute force method (sending rays in a lot of directions) for computing k.
*/
float unboundQuadricBruteForce(in vec3 p, in vec3 n, in float dist)
{
	float k = 2;

	for (int i = 0; i < uSampleResolution.x; ++i)
	{
		for (int j = 0; j <= uSampleResolution.y; ++j)
		{
			// N*M rays using spherical coordinates
			float u = 2 * PI * i / float(uSampleResolution.x);
			float v = PI * j / float(uSampleResolution.y);
			vec3 d = normalize(vec3(cos(u) * sin(v), cos(v), sin(u) * sin(v)));
			Ray r = Ray(p, 0.0, d, 100);
			
			TraceResult res = sphereTrace(r, SphereTraceDesc(0.01, 100));
			if (bool(res.flags & 1) || bool(res.flags & 4)) { continue; }

			// if hits the surface, evaluate minimal k
			float scale = res.T;
			float cosPhi = dot(d, n);
			float sinPhi = length(cross(d, n));
			vec2 pos2d = vec2(sinPhi, cosPhi) * scale;
			k = min(k, mix(quadric_ComputeParameter(pos2d), -1.0, uCorrection));
		}
	}

	return (k > 1 ? -1 : k);
}

float unboundQuadricConeTrace(in vec3 p, in vec3 n, in float dist)
{
	float k = 1;

	int ray_count = RAY_DIRECTIONS.length();

	// default cone trace desc
	ConeTraceDesc coneTraceDes = ConeTraceDesc(0.01f, 100, RayCone(Ray(vec3(0.0f), 0.0f, vec3(0.0f), 0.0f), 0.0f, 0.0f), n);
	for (int i = 0; i < ray_count; i++)
	{
		RayCone r = RayCone(Ray(p,
			0.0,
			normalize(RAY_DIRECTIONS[i]),
			100), RAY_HALF_TANGENTS[i], 0.0f);

		coneTraceDes.ray = r;
		ConeTraceResult res = cone_trace(coneTraceDes);
		if (bool(res.flags & 1) || bool(res.flags & 4)) { continue; }

		vec3 ray_dir = normalize(res.p - p);

		// if hits the surface, evaluate minimal k
		float scale = length(res.p - p);
		float cosPhi = dot(ray_dir, n);
		float sinPhi = length(cross(ray_dir, n));
		vec2 pos2d = vec2(sinPhi, cosPhi) * scale;
		k = min(k, quadric_ComputeParameter(pos2d));
	}

	return -k;
}

void main()
{
	ivec3 coords = ivec3(gl_GlobalInvocationID.xyz);
	vec3 p = voxelToGlobal(coords, ivec3(gl_NumWorkGroups.xyz));

	QuadricField ret;
	ret.dist = SDF(p);
	ret.normal = -computeGradient(p);
	ret.k = UNBOUND_QUADRIC(p, ret.normal, ret.dist);
	
	storeQuadricField(outField, coords, ret);
}
