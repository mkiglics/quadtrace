#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

restrict writeonly uniform image3D ecc;
uniform sampler3D sdf_values;

uniform int N = 70;
uniform int M = 70;
uniform float correction = 0.01;

#define EPSILON 0.01
#define PI 3.14159265359

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
		d	  > params.epsilon&&		// Stop if cone is close to surface
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
			TraceResult res = sphere_trace(r, SphereTraceDesc(1, 100), ivec3(gl_NumWorkGroups.xyz)+ivec3(1));
			if (bool(res.flags & 1) || bool(res.flags & 4)) { continue; }

			// if hits the surface, evaluate minimal k
			float scale = res.T;
			float cosPhi = dot(d, norm);
			float sinPhi = length(cross(d, norm));
			vec2 pos2d = vec2(sinPhi, cosPhi) * scale;
			k = min(k, mix(getK(pos2d), -1.0, correction));
		}
	}

	imageStore(ecc, coords, vec4(k));
}
































