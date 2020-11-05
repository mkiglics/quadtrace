float SDF(vec3 p)
{
	return Substract(torus(p, 4, 1), sphere(p, 4));
}