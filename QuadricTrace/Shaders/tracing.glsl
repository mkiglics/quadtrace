#version 450

#define SGN(a) (a < 0 ? -1 : 1)
//#define S 1.0

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

/* Numerically stable quadratic equation solver */
vec2 solveQuadratic(float a, float b, float c)
{
	float d = b*b-4*a*c;
	float t1 = (-b-SGN(b)*sqrt(d))/(2*a);
	float t2 = c/(a*t1);
	return d<0 ? vec2(-inf, inf) : vec2(min(t1,t2), max(t1,t2));
}

// returns the quadric's parameter
float getK(vec2 pos)
{	
	return pos.y>0 ? solveQuadratic(pos.x*pos.x, 2*pos.y*pos.y-pos.y, -pos.y*pos.y).y : solveQuadratic(pos.x*pos.x, -2*pos.y*pos.y-pos.y, -pos.y*pos.y).x;
}
