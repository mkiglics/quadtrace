float SDF(vec3 p) {
	return add(sphere(p, vec3(0), 6), box(p, vec3(5)));
}