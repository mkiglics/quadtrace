float SDF(vec3 p) {
	return sub(sphere(p, vec3(1,1,1), 5), box(p, vec3(5)));
}