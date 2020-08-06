float SDF(vec3 p) {
	float a = Union(sphere(p, vec3(0, 2, 0), 3), box(p,vec3(2,3,2)));
	float b = Union(torus(p-vec3(-2,2,2), 2, 1.5), torus(p-vec3(2,2,-2), 2, 1.5));
	float c = box(p, vec3(2, 5, 2));
	float d = cylinder(p, 2.5, 5);

	return Intersect(d, Sub(b,Intersect(a,c)));
}