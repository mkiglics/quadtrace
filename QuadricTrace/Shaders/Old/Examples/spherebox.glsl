float SDF(vec3 p) {
	return Substract(box(p, vec3(5)), Offset(sphere(p, 5), 1));
}