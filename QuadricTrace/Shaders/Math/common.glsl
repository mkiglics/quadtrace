//?#version 450

const float inf = 1. / 0.; // at least OpenGL 4.1

/*
	Returns a rotation matrix for rotating normal to (0, length(normal), 0)
	using Rodrigues' formula
*/
mat3 getRotation(vec3 normal)
{
	vec3 y = vec3(0, 1, 0);
	vec3 v = cross(normal, y);
	float c = dot(normal, y);
	if (c < -0.99) {
		return mat3(1, 0, 0, 0, -1, 0, 0, 0, -1);
	}
	mat3 m = mat3(0, v.z, -v.y, -v.z, 0, v.x, v.y, -v.x, 0);
	return mat3(1) + m + m * m / (1 + c);
}
