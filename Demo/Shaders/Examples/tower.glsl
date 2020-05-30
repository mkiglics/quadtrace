float SDF(vec3 p) {
	float a = add(sphere(p, vec3(0, 2, 0), 3), box(p,vec3(2,3,2)));
	float b = add(torus(p-vec3(-2,2,2), vec2(2,1.5)), torus(p-vec3(2,2,-2), vec2(2,1.5)));
	float c = box(p, vec3(2, 5, 2));
	float d = cylinder(p, 2.5, 5);

	return intersect(d, sub(b,intersect(a,c)));
}