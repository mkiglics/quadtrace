float SDF(vec3 p)
{
	return sub(sphere(p, vec3(0), 4), torus(p, vec2(4, 1)));
}