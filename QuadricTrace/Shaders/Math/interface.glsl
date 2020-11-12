//?#version 450

uniform sampler3D samplerValues;
//restrict writeonly uniform image3D in_quadric_field;
//restrict readonly uniform image3D out_quadric_field;

uniform vec3 uAabbCorner = vec3(-5, -5, -5);
uniform vec3 uAabbSize = vec3(10, 10, 10);

//converts texel coordinates (int in range [0, n-1]) to global floats
vec3 voxelToGlobal(ivec3 p, ivec3 grid_size)
{
	return vec3(p)/vec3(grid_size) * uAabbSize + uAabbCorner;
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

/*
 * How big a voxel is in the world.
*/
vec3 voxelWorldSize(ivec3 grid_size)
{
	return uAabbSize / grid_size;
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

QuadricField loadQuadricField(layout(rgba32f) readonly image3D in_field, vec3 world_pos)
{
	ivec3 coords = globalToVoxel(world_pos, imageSize(in_field));
	return decodeQuadricField(imageLoad(in_field, coords));
}

/*
 * Given a world coordianate it returns an interpolated QuadricField based on the surrounding
 * stored QuadricFields.
*/
QuadricField interpolateQuadricField(layout(rgba32f) image3D in_field, vec3 world_pos)
{
	// given world pos is outside of the grid
	if (any(lessThan(world_pos, uAabbCorner)) || any(greaterThanEqual(world_pos, uAabbCorner + uAabbSize))) 
	{
		return loadQuadricField(in_field, world_pos);
	}

	QuadricField qField = QuadricField(0.0f, 0.0f, vec3(0.0f));
	ivec3 gridSize = imageSize(in_field);
	ivec3 lowestGridPoint = globalToVoxel(world_pos, gridSize);

	// TODO

	return qField;
}

void storeQuadricField(restrict writeonly image3D out_field, ivec3 coords, QuadricField field)
{
	imageStore(out_field, coords, encodeQuadricField(field));
}

// length(normal) = k, where k needs to be > 0

