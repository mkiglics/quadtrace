#version 450

float SDF(vec3 p)
{
	vec3 q = abs(p) - vec3(2, 3, 4);
	return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
	return length(p + vec3(0, -0, 0)) - 5;
}

