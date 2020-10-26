#version 450

const float inf = 1. / 0.; // at least OpenGL 4.1

struct Ray
{
	vec3 P;
	float Tmin;
	vec3 V;
	float Tmax;
};

struct TraceResult
{
	float T;		// Distance taken on ray
	int flags;		// bit 0:   distance condition:     true if travelled to far t > t_max
					// bit 1:   surface condition:      true if distance to surface is small < error threshold
};                  // bit 2:   iteration condition:    true if took too many iterations

struct SphereTraceDesc
{
	float epsilon;  //Stopping distance to surface
	int maxiters;   //Maximum iteration count
};

//converts texel coordinates (int in range [0, n-1]) to global floats
vec3 texelToGlobal(ivec3 p, ivec3 n)
{
	return (p-(n-vec3(1))/2.0);
}

//converts global float coordinates to texel postions
ivec3 globalToTexel(vec3 p, ivec3 n)
{
	return clamp(ivec3(round(p + (n-vec3(1))/2.0)), ivec3(0), ivec3(n-ivec3(1)));
}