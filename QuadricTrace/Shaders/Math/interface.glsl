//?#version 450

uniform sampler3D samplerValues;
//restrict writeonly uniform image3D in_quadric_field;
//restrict readonly uniform image3D out_quadric_field;

uniform vec3 uAabbCorner;
uniform vec3 uAabbSize;

//converts texel coordinates (int in range [0, n-1]) to global floats
vec3 voxelToGlobal(ivec3 p, ivec3 grid_size)
{
	return vec3(p)/vec3(grid_size)*uAabbSize + uAabbCorner;
	//return p - (grid_size - vec3(1.0)) / 2.0;
}

//converts global float coordinates to voxel postions
ivec3 globalToVoxel(vec3 p, ivec3 grid_size)
{
	vec3 size = ivec3(grid_size);
	return ivec3(clamp(floor((p-uAabbCorner)/uAabbSize*size), vec3(0), size));
	//return clamp(ivec3(round((p-uAabbCorner)/uAabbSize*grid_size)), ivec3(0), grid_size);
	//return clamp(ivec3(round(p + (grid_size - vec3(1.0)) / 2.0)), ivec3(0.0), ivec3(grid_size - ivec3(1.0)));
}

struct QuadricField 
{
	float k;
	float dist;
	vec3 normal;
};

vec4 encodeQuadricField(in QuadricField values) 
{
	return vec4(values.dist, (values.k + 2.0f) * values.normal);
}

QuadricField decodeQuadricField(in vec4 encoded) 
{
	float normalLength = length(encoded.yzw);
	return QuadricField(normalLength - 2.0f, encoded.x, encoded.yzw / normalLength);
}

QuadricField loadQuadricField(layout(rgba32f) image3D in_field, vec3 world_pos)
{
	ivec3 coords = globalToVoxel(world_pos, imageSize(in_field));
	return decodeQuadricField(imageLoad(in_field, coords));
}

void storeQuadricField(image3D out_field, ivec3 coords, QuadricField field)
{
	imageStore(out_field, coords, encodeQuadricField(field));
}

// length(normal) = k, where k needs to be > 0

