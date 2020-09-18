#pragma once

#include "../expr.h"
#include <string>

struct Fields { int material; };
using MyExpr = Expr<Fields>;

struct Model2Attributes {
	float xSize = 1.0f;
	float ySize = 0.3f;
	float zSize = 0.5f;
	int xRidgeCount = 6;
	int yRidgeCount = 3;
};

MyExpr* model1_expr(float cylinderRadius = 0.25f, float baseSize = 0.8f, float baseHeight = 0.2f);
MyExpr* model2_expr(Model2Attributes attr = Model2Attributes{});
MyExpr* model3_expr(float r = 1.0f, float height = 0.3f, int holeCount = 4);
MyExpr* model4_expr(float width = 1.0f, float height = 1.0f);
MyExpr* model5_expr(float width = 1.0f, float height = 1.0f, float holeRadius = 0.1f);
MyExpr* model7_expr(float boxWidth = 1.0f, float boxHeight = 1.0f, int N = 3, int M = 3);
MyExpr* model8_expr(int N = 2, int M = 2, int connectTubeCount = 3, Model2Attributes attr = Model2Attributes{});
MyExpr* model9_expr(float r = 1.0f, float height = 0.3f, int holeCount = 4, int N = 3, int M = 3);

void build_kernel(const std::string& file_name, MyExpr* expr);
