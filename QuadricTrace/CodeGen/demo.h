#pragma once

#include "expr.h"
#include <string>

struct MyFields { int material; };
using MyExpr = Expr<MyFields>;

MyExpr* demo_expr();
MyExpr* demo2();

MyExpr* toyCube(int mat);
MyExpr* toyBrickYthin(int mat);
MyExpr* toyBrickXthin(int mat);
MyExpr* toyBrickYthick(int mat);
MyExpr* toyBrickXthick(int mat);
MyExpr* toyCylinder(int mat);
MyExpr* toyCylinderSmall(int mat);
MyExpr* toyArch(int mat);
MyExpr* toyRoof(int mat);

void build_kernel(const std::string &file_name, MyExpr* expr);
void build_footmap(const std::string &file_name, MyExpr* expr);