float SDF(in vec3 p) {
	float d = sphere(p, 1.0);
    int n = 10;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                d = Union(d, sphere(p - vec3(i,j,k), 1.0));
            }
        }
    }

    return d;
}