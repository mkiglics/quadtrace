#define GLM_FORCE_SWIZZLE
#define _USE_MATH_DEFINES
#include "demo.h"
#include "bounding_opt.h"
#include "codegen.h"
#include "Footmap.h"
#include "material.h"
#include "glm/glm.hpp"
#include <cmath>
#include <fstream>

using namespace std;
using namespace glm;

const float g_offset = 0.25f;
MyExpr* toyCube(int mat) { return offset(g_offset, move({ 0.f, 1.f, 0.f }, box(0.9f, 0.9f, 0.9f, MyFields{ mat }))); }

MyExpr* toyBrickYthin(int mat) { return offset(g_offset, move({ 0.f, 3.f, 0.f }, box(0.4f, 2.9f, 0.9f, MyFields{ mat }))); }

MyExpr* toyBrickXthin(int mat) { return offset(g_offset, move({ 0.f, 0.5f, 0.f }, box(2.9f, 0.4f, 0.9f, MyFields{ mat }))); }

MyExpr* toyBrickYthick(int mat) { return offset(g_offset, move({ 0.f, 3.f, 0.f }, box(0.9f, 2.9f, 0.9f, MyFields{ mat }))); }

MyExpr* toyBrickXthick(int mat) { return offset(g_offset, move({ 0.f, 1.f, 0.f }, box(2.9f, 0.9f, 0.9f, MyFields{ mat }))); }
//MyExpr* toyBrickXthick(int mat) { return move({ 0.f, 1.f, 0.f }, box(2.9f, 0.9f, 0.9f, MyFields{ mat })); }

MyExpr* toyCylinder(int mat) { return offset(g_offset, move({ 0.f, 3.f, 0.f }, cylinder(Dir1D::Y, 0.9f, 2.9f, MyFields{ mat }))); }

MyExpr* toyCylinderSmall(int mat) { return offset(g_offset, move({ 0.f, 1.f, 0.f }, cylinder(Dir1D::Y, 0.9f, 0.9f, MyFields{ mat }))); }

MyExpr* toyArch(int mat) { return offset(g_offset, subtract<MyFields>(toyBrickXthick(mat), move({ 0.f, -1.4f, 0.f }, cylinder(Dir1D::Z, 2.6f, MyFields{ mat })))); }

MyExpr* toyRoof(int mat) {
    return offset(g_offset,
        intersect<MyFields>(toyBrickXthick(mat),
        move({ 0.f, -1.5f, 0.f }, cylinder(Dir1D::Z, 3.3f, 0.9f, MyFields{ mat }))));
}

mat3 fitToyY(vec2 a, vec2 b) {
    vec2 v = normalize(b - a);
    return mat3(v.x, 0, -v.y, 0, 1, 0, v.y, 0, v.x);
}

MyExpr* demo_expr() {
    vec3 p1(0, 0, 0), p2(4, 0, 0);
    mat3 M12 = fitToyY(p1.xz, p2.xz);
    vec3 p3 = 4.0f * normalize(vec3(-1, 0, 2));
    mat3 M13 = fitToyY(p1.xz, p3.xz);
    mat3 M1 = rotateY(0.5);
    mat3 M2 = rotateY((float)-M_PI * 0.5f);
    mat3 M3 = rotateY(0.4f);
    mat3 M4 = rotateZ(0.42f);
    mat3 M5 = rotateY(0.123f);
    mat3 M1t = transpose(M1);
    return
        union_op<MyFields>(
            planeXZ(MyFields{ 0 }),
            move(p1 + vec3(-0.4, 0, 0.4), rotate(M1t, toyCube(2))),
            move(p2 + vec3(-0.1, 0, 1.4), rotate(M2, toyBrickXthick(4))),
            move(p1 + vec3(0, 4, 0), rotate(M5, toyCube(1))),
            move(p3, toyCylinder(3)),
            move(0.5f * (p1 + p2) + vec3(0, 2, 0), rotate(M12, toyArch(1))),
            move(0.5f * (p1 + p3) + vec3(0, 6, 0), rotate(M13, toyRoof(3))),
            move(vec3(4, 4, 0), toyCylinderSmall(4)),
            move(vec3(8.5, 0, 1.8), rotate(M5, toyBrickYthick(2))),
            move(vec3(6.5, 6, 1), rotate(M3, toyBrickXthin(3))),
            move(vec3(6.625, 1.193, 4.036), rotate(M4, toyBrickXthin(4))),
            move(vec3(5, 1, 6.5), sphere(1, MyFields{ 1 })),
            move(vec3(1, 0, 6), rotate(M13, toyCube(2))));
}

MyExpr* demo2(){return toyArch(1);}

const string hlsl_prefix =
R"(#ifndef SDF_HLSL
#define SDF_HLSL

#include "SDF/primitives.hlsl"

)";
const string hlsl_postfix = "\n#endif\n";

const string glsl_prefix = "\n// Start of generated GLSL code\n";
const string glsl_postfix = "\n// End of generated GLSL code\n";

void build_kernel(const string &file_name, MyExpr* expr) {
    std::ofstream kernel(file_name);
    kernel << glsl_prefix << sdf(*expr) << /*"\n" << material(*expr) <<*/ glsl_postfix;
}

void build_footmap(const string &file_name, MyExpr* expr) {
    std::ofstream kernel(file_name);
    kernel << glsl_prefix << footmap(*expr) << glsl_postfix;
}

