#version 450

float sphere(vec3 p, vec3 c, float r) {
	return length(p - c) - r;
}

float box(vec3 p, vec3 b) 
{
	vec3 q = abs(p) - b;
	return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float add(float d1, float d2) 
{
	return min(d1,d2);
}

float sub(float d1, float d2) 
{
	return max(-d1,d2);
}

float intersect(float d1, float d2)
{
	return max(d1,d2);
}

float SDF(vec3 p)
{
	//return sphere(p, vec3(0), 5);
	return add(sphere(p, vec3(0,6,0), 2),sub(box(p, vec3(4.3,3.2,1)), add(sphere(p, vec3(1, 0, 0), 3), sphere(p, vec3(-5, 0, 0), 4))));
}



