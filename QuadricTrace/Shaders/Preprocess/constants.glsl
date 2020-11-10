#version 450

#define PHI 1.61803398874989484820458683436 

#ifndef RAY_DIRECTIONS
#define RAY_DIRECTIONS gRay6Directions
#endif

#ifndef RAY_HALF_TANGENTS
#define RAY_HALF_TANGENTS gRay6HalfTangents
#endif

// Cone tracing tetrahedron 
const vec3 gRay4Directions[]= { 
	vec3(1, 1, -1) / sqrt(3), 
	vec3(-1, -1, -1) / sqrt(3), 
	vec3(1, -1, 1) / sqrt(3), 
	vec3(-1, 1, 1) / sqrt(3) 
}; 
const float gRay4HalfTangents[] = { 
	2 * sqrt(2), 2 * sqrt(2), 
	2 * sqrt(2), 2 * sqrt(2) 
};

// Cone tracing cube
const vec3 gRay6Directions[]= { 
	vec3(0, 0, 1), 
	vec3(0, 0, -1), 
	vec3(0, 1, 0), 
	vec3(0, -1, 0), 
	vec3(1, 0, 0), 
	vec3(-1, 0, 0), 
}; 
const float gRay6HalfTangents[] = { 
	sqrt(2), sqrt(2), 
	sqrt(2), sqrt(2), 
	sqrt(2), sqrt(2) 
};

// Cone tracing octahedron
const vec3 gRay8Directions[]= { 
	vec3(1, 1, 1) / sqrt(3), 
	vec3(-1, -1, -1) / sqrt(3), 
	vec3(-1, 1, 1) / sqrt(3), 
	vec3(1, -1, 1) / sqrt(3), 
	vec3(1, 1, -1) / sqrt(3), 
	vec3(-1, -1, 1) / sqrt(3), 
	vec3(1, -1, -1) / sqrt(3), 
	vec3(-1, 1, -1) / sqrt(3)
}; 
const float gRay8HalfTangents[] = { 
	2 / sqrt(2), 2 / sqrt(2), 2 / sqrt(2), 2 / sqrt(2), 
	2 / sqrt(2), 2 / sqrt(2), 2 / sqrt(2), 2 / sqrt(2) 
};

// Cone tracing icosahedron
const float ICO_TAN = 4 / (3 + sqrt(5)); 
uniform vec3 gRay20Directions[]= { 
	vec3( (1 + PHI),  (1 + PHI),  (1 + PHI)), 
	vec3(-(1 + PHI),  (1 + PHI),  (1 + PHI)), 
	vec3( (1 + PHI), -(1 + PHI),  (1 + PHI)), 
	vec3( (1 + PHI),  (1 + PHI), -(1 + PHI)), 
	vec3(-(1 + PHI), -(1 + PHI),  (1 + PHI)), 
	vec3( (1 + PHI), -(1 + PHI), -(1 + PHI)), 
	vec3(-(1 + PHI),  (1 + PHI), -(1 + PHI)), 
	vec3(-(1 + PHI), -(1 + PHI), -(1 + PHI)), 
	vec3( PHI, 0, 1 +  2 * PHI), 
	vec3(-PHI, 0, 1 +  2 * PHI), 
	vec3( PHI, 0, 1 + -2 * PHI), 
	vec3(-PHI, 0, 1 + -2 * PHI), 
	vec3(0, -PHI, 1 + -2 * PHI), 
	vec3(0,  PHI, 1 + -2 * PHI), 
	vec3(0, -PHI, 1 +  2 * PHI), 
	vec3(0,  PHI, 1 +  2 * PHI), 
	vec3( PHI, 1 +  2 * PHI, 0), 
	vec3(-PHI, 1 +  2 * PHI, 0), 
	vec3( PHI, 1 + -2 * PHI, 0), 
	vec3(-PHI, 1 + -2 * PHI, 0) 
}; 
uniform float gRay20HalfTangents[] = { 
	ICO_TAN, ICO_TAN, ICO_TAN, ICO_TAN, 
	ICO_TAN, ICO_TAN, ICO_TAN, ICO_TAN, 
	ICO_TAN, ICO_TAN, ICO_TAN, ICO_TAN, 
	ICO_TAN, ICO_TAN, ICO_TAN, ICO_TAN, 
	ICO_TAN, ICO_TAN, ICO_TAN, ICO_TAN 
};
