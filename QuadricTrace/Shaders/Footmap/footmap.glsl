
// Start of generated GLSL code

vec4 f0(vec3 p){
	vec4 r0 = fm_planeY(p);
	return r0;
}

vec4 f1(vec3 p){
	vec4 r0 = fm_box(mat3(0.877583,0,0.479426,0,1,0,-0.479426,0,0.877583)*p - vec3(-0.542803,1,0.159263), vec3(0.9,0.9,0.9));
	r0 -= 0.1 * vec4(normalize(r0.xyz),1); //offset
	return r0;
}

vec4 f2(vec3 p){
	vec4 r0 = fm_box(mat3(-4.37114e-08,0,1,0,1,0,-1,0,-4.37114e-08)*p - vec3(-1.4,1,3.9), vec3(2.9,0.9,0.9));
	return r0;
}

vec4 f3(vec3 p){
	vec4 r0 = fm_box(mat3(0.992445,0,-0.12269,0,1,0,0.12269,0,0.992445)*p - vec3(0,5,0), vec3(0.9,0.9,0.9));
	r0 -= 0.1 * vec4(normalize(r0.xyz),1); //offset
	return r0;
}

vec4 f4(vec3 p){
	vec4 r0 = fm_cylinderY(p - vec3(-1.78885,3,3.57771), vec2(0.9,2.9));
	r0 -= 0.1 * vec4(normalize(r0.xyz),1); //offset
	return r0;
}

vec4 f5(vec3 p){
	vec4 r0 = fm_box(p - vec3(2,3,0), vec3(2.9,0.9,0.9));
	return r0;
}

vec4 f6(vec3 p){
	vec4 r0 = fm_cylinderZ(p - vec3(2,0.6,0), 2.6);
	r0.w *= -1.f;
	return r0;
}

vec4 f7(vec3 p){
	vec4 r0 = vec4(0);
	vec4 r1 = f5(p);
	vec4 r2 = f6(p);
	vec4 r3 = f6(p + r1.xyz);
	vec4 r4 = f5(p + r2.xyz);
	vec4 r5 = r1.w > r2.w ? r1 : r2;
	r0.xyz = r5.w<=0. ? r5.xyz : r0.xyz;
	r0.xyz = r5.w>0.0 && r3.w<0.001 ? r1.xyz : r0.xyz;
	r0.xyz = r5.w>0.0 && r4.w<0.001 ? r2.xyz : r0.xyz;
	vec3 r6 = 0.5 * (r1.xyz + r2.xyz + r4.xyz + r3.xyz);
	r0.xyz = r5.w>0.0 && r4.w>=0.001 && r3.w>=0.001 ? r6 : r0.xyz;
	r0.w = (r5.w<0.?-1.:1.) * length(r0.xyz);
	return r0;
}

vec4 f8(vec3 p){
	vec4 r0 = fm_box(mat3(-0.447214,0,-0.894427,0,1,0,0.894427,0,-0.447214)*p - vec3(2,7,0), vec3(2.9,0.9,0.9));
	return r0;
}

vec4 f9(vec3 p){
	vec4 r0 = fm_cylinderZ(mat3(-0.447214,0,-0.894427,0,1,0,0.894427,0,-0.447214)*p - vec3(2,4.5,0), vec2(3.3,0.9));
	r0 -= 0.1 * vec4(normalize(r0.xyz),1); //offset
	return r0;
}

vec4 f10(vec3 p){
	vec4 r0 = vec4(0);
	vec4 r1 = f8(p);
	vec4 r2 = f9(p);
	vec4 r3 = f9(p + r1.xyz);
	vec4 r4 = f8(p + r2.xyz);
	vec4 r5 = r1.w > r2.w ? r1 : r2;
	r0.xyz = r5.w<=0. ? r5.xyz : r0.xyz;
	r0.xyz = r5.w>0.0 && r3.w<0.001 ? r1.xyz : r0.xyz;
	r0.xyz = r5.w>0.0 && r4.w<0.001 ? r2.xyz : r0.xyz;
	vec3 r6 = 0.5 * (r1.xyz + r2.xyz + r4.xyz + r3.xyz);
	r0.xyz = r5.w>0.0 && r4.w>=0.001 && r3.w>=0.001 ? r6 : r0.xyz;
	r0.w = (r5.w<0.?-1.:1.) * length(r0.xyz);
	return r0;
}

vec4 f11(vec3 p){
	vec4 r0 = fm_cylinderY(p - vec3(4,5,0), vec2(0.9,0.9));
	r0 -= 0.1 * vec4(normalize(r0.xyz),1); //offset
	return r0;
}

vec4 f12(vec3 p){
	vec4 r0 = fm_box(mat3(0.992445,0,-0.12269,0,1,0,0.12269,0,0.992445)*p - vec3(8.65663,3,0.743535), vec3(0.9,2.9,0.9));
	r0 -= 0.1 * vec4(normalize(r0.xyz),1); //offset
	return r0;
}

vec4 f13(vec3 p){
	vec4 r0 = fm_box(mat3(0.921061,0,-0.389418,0,1,0,0.389418,0,0.921061)*p - vec3(6.37631,6.5,-1.61016), vec3(2.9,0.4,0.9));
	r0 -= 0.1 * vec4(normalize(r0.xyz),1); //offset
	return r0;
}

vec4 f14(vec3 p){
	vec4 r0 = fm_box(mat3(0.913089,0.40776,0,-0.40776,0.913089,0,0,0,1)*p - vec3(5.35888,4.24727,4.036), vec3(2.9,0.4,0.9));
	r0 -= 0.1 * vec4(normalize(r0.xyz),1); //offset
	return r0;
}

vec4 f15(vec3 p){
	vec4 r0 = fm_sphere(p - vec3(5,1,6.5), 1);
	return r0;
}

vec4 f16(vec3 p){
	vec4 r0 = fm_box(mat3(-0.447214,0,-0.894427,0,1,0,0.894427,0,-0.447214)*p - vec3(4.91935,1,-3.57771), vec3(0.9,0.9,0.9));
	r0 -= 0.1 * vec4(normalize(r0.xyz),1); //offset
	return r0;
}

vec4 footmap(vec3 p)
{
	vec4 r0 = f0(p);
	vec4 r1 = vec4(0);
	r1 = f1(p);
	r0 = r0.w < r1.w ? r0 : r1;
	r1 = f2(p);
	r0 = r0.w < r1.w ? r0 : r1;
	r1 = f3(p);
	r0 = r0.w < r1.w ? r0 : r1;
	r1 = f4(p);
	r0 = r0.w < r1.w ? r0 : r1;
	r1 = f7(p);
	r0 = r0.w < r1.w ? r0 : r1;
	r1 = f10(p);
	r0 = r0.w < r1.w ? r0 : r1;
	r1 = f11(p);
	r0 = r0.w < r1.w ? r0 : r1;
	r1 = f12(p);
	r0 = r0.w < r1.w ? r0 : r1;
	r1 = f13(p);
	r0 = r0.w < r1.w ? r0 : r1;
	r1 = f14(p);
	r0 = r0.w < r1.w ? r0 : r1;
	r1 = f15(p);
	r0 = r0.w < r1.w ? r0 : r1;
	r1 = f16(p);
	r0 = r0.w < r1.w ? r0 : r1;
	r0.w = (r0.w > 0. ? 1. : -1.) * length(r0.xyz);
	return r0;
}
float SDF(vec3 p)
{
	vec4 m = footmap(p);
	return length(m.xyz)*(m.w>0.?1.:-1.);
}

// End of generated GLSL code
