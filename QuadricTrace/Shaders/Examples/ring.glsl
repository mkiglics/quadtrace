float SDF(vec3 p)
{
	return Sub(sphere(p, vec3(0), 4), torus(p, 4, 1));
}