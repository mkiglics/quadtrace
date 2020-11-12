//?#version 450

//?#include "../SDF/SDFcommon.glsl"

/*
 * Stores the ray data that is emitted from a pixel of the camera
*/
struct DistanceField 
{
	vec3 hitPoint;
	float dist;
};

vec4 encodeDistanceField(in DistanceField values) 
{
	return vec4(values.hitPoint, values.dist);
}

DistanceField decodeDistanceField(in vec4 encoded) 
{
	return DistanceField(encoded.xyz, encoded.w);
}

DistanceField loadDistanceField(layout(rgba32f) readonly image2D in_field, ivec2 pixel_pos)
{
	return decodeDistanceField(imageLoad(in_field, pixel_pos));
}

void storeDistanceField(restrict writeonly image2D out_field, ivec2 coords, DistanceField field)
{
	imageStore(out_field, coords, encodeDistanceField(field));
}
