#version 450
//!#include "Primitives/common.glsl"

//convex optimized
//must be exactly the same implementation, but with vec2 and passing v around
//if you rotate p, don't forget to rotate v too
//vec2 sdf(vec3 p, vec3 v);

//#include "SDF/primitives_demo.hlsl"
//#include "SDF/mandelbulb.hlsl"
//#include "SDF/simple_test.hlsl"
//#include "SDF/spheres.hlsl"


mat3 Rotate(vec3 k, float angle)
{
   float c = cos(angle);
   vec3 s = sin(angle) * k;
   vec3 diag = (1 - c) * k * k + c;
   vec3 tk = (1 - c) * k.xyz * k.yzx;

   return mat3(
       diag.x,       tk.x+s.z,     tk.z-s.y,
       tk.x-s.z,     diag.y,       tk.y+s.x,
       tk.z+s.y,     tk.y-s.x,     diag.z
   );
}

mat3 RotateX(float angle)
{
   float s = sin(angle), c = cos(angle);
   	return mat3(1, 0, 0, 0, -c, s, 0, s, c);
}

mat3 RotateY(float angle)
{
   float s = sin(angle), c = cos(angle);
   return mat3(c, 0, s, 0, 1, 0, -s, 0, c);
}

mat3 RotateZ(float angle)
{
   float s = sin(angle), c = cos(angle);
   return mat3(c, -s, 0, s, c, 0, 0, 0, 1);
}

float toyCube(vec3 p)
{
    return Offset(box(p - vec3(0,1,0), vec3(0.9, 0.9, 0.9)), 1.0);
}

float toyBrickYthin(vec3 p)
{
    return Offset(box(p - vec3(0, 3, 0), vec3(0.4, 2.9, 0.9)), 1.0);
}

float toyBrickXthin(vec3 p)
{
    return Offset(box(p - vec3(0, 0.5, 0), vec3(2.9, 0.4, 1)), 1.0);
}

float toyBrickYthick(vec3 p)
{
    return Offset(box(p - vec3(0, 3, 0), vec3(0.9, 2.9, 0.9)), 1.0);
}

float toyBrickXthick(vec3 p)
{
    return Offset(box(p - vec3(0, 1.0, 0), vec3(3, 0.9, 0.9)), 1.0);
}

float toyCylinder(vec3 p)
{
    return Offset(cylinderY(p - vec3(0, 3, 0), vec2(0.9, 2.9)), 1.0);
}

float toyCylinderSmall(vec3 p)
{
    return Offset(cylinderY(p - vec3(0, 1, 0), vec2(0.9, 0.9)), 1.0);
}

float toyArch(vec3 p)
{
    return Substract(toyBrickXthick(p), cylinderZ(p - vec3(0, -1.4, 0), 3.0));
}

float toyRoof(vec3 p)
{
    return Intersect(toyBrickXthick(p),
        Offset(cylinderZ(p - vec3(0, -1.5, 0), vec2(3, 1)),1.0));
}

//rotates a piace such that it fits a and b: useful for long ones
mat3 fitToyY(vec2 a, vec2 b)
{
    vec2 v = normalize(b - a);
    return mat3(v.x, 0, -v.y, 0, 1, 0, v.y, 0, v.x);
}

float SDF(vec3 p)
{
	
    float d = 1000;
	
    vec3 p1 = vec3(0,0,0);
    vec3 p2 = vec3(4,0,0);
    mat3 M12 = fitToyY(p1.xz, p2.xz);
    vec3 p3 = 4 * normalize(vec3(-1, 0,2));
    mat3 M13 = fitToyY(p1.xz, p3.xz);
    mat3 M1 = RotateY(0.5);
    mat3 M2 = RotateY(-PI*0.5);
	mat3 M3 = RotateY(0.4);
	mat3 M4 = RotateZ(0.42);
	mat3 M5 = RotateY(0.123);
    mat3 M1t = transpose(M1);
	//mat3 M = Rotate(gRectLightNorm, gRectLightAngle);

    d = Union(d, toyCube((p - p1 - vec3(-0.4,0,0.4))* M1t));
    d = Union(d, toyBrickXthick((p - p2 - vec3(-0.1,0,1.4))* M2));
    d = Union(d, toyCube((p - p1 - vec3(0, 4, 0))* M5));
    d = Union(d, toyCylinder(p - p3));
    d = Union(d, toyArch((p - 0.5 * (p1 + p2) - vec3(0, 2, 0))* M12));
    d = Union(d, toyRoof((p - 0.5 * (p1 + p3) - vec3(0, 6, 0)) * M2 * M3));
	d = Union(d, toyCylinderSmall(p - vec3(4,4,0)));
    d = Union(d, toyBrickYthick((p - vec3(8.5,0,1.8)) * M5));
    d = Union(d, toyBrickXthin((p - vec3(6.5,6,1)) * M3));
    d = Union(d, toyBrickXthin((p - vec3(6.625,1.193,4.036)) * M4));
    d = Union(d, sphere(p - vec3(5,1,6.5), 1));
    d = Union(d, toyCube((p - vec3(1, 0, 6)) * M13));

    return d;
	
}