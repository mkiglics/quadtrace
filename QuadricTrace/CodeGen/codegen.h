#pragma once

#include "expr.h"
#include "util.h"
#include "glm/glm.hpp"
#include <array>
#include <string>

template<typename Fields>
struct CodeGen {

    struct Carrier {
        std::string code, reg;
    };

    struct State {
        int next_reg        = 0;
        glm::vec3 move      = glm::vec3(0);
        glm::mat3 rotate    = glm::mat3(1);
    } state;

    Carrier operator()(Box<Fields> expr) {
        return assign_register("box(" + move_rotate() + ", " + float3(expr.x, expr.y, expr.z) + ");\n");
    }
    Carrier operator()(Sphere<Fields> expr) {
        return assign_register("sphere(" + move_rotate() + ", " + float1(expr.r) + ");\n");
    }
    Carrier operator()(Cylinder<Fields> expr) {
        using namespace std::string_literals;
        return assign_register("cylinder"s + (char)expr.dir + "(" + move_rotate() + ", " +
            (expr.y == std::numeric_limits<float>::infinity() ? float1(expr.x) : float2(expr.x, expr.y)) + ");\n");
    }
    Carrier operator()(PlaneXZ<Fields> expr) {
        return assign_register("planeXZ(" + move_rotate() + ");\n");
    }
    Carrier operator()(Offset<Fields> expr) {
        auto sub_expr = visit(*this, *expr.a);
        sub_expr.code += sub_expr.reg + " -= " + float1(expr.r) + ";\n";
        return sub_expr;
    }
    Carrier operator()(Intersect<Fields> expr) {
        auto result = visit(*this, *expr.a[0]);
        for (int i = 1; i < (int)expr.a.size(); ++i) {
            auto sub_expr = visit(*this, *expr.a[i]);
            result.code += sub_expr.code + result.reg + " = " + "max(" + result.reg + ',' + sub_expr.reg + ");\n";
        }
        return result;
    }
	Carrier operator()(Union<Fields> expr) {
        auto result = visit(*this, *expr.a[0]);
        for (int i = 1; i < (int)expr.a.size(); ++i) {
            auto sub_expr = visit(*this, *expr.a[i]);
            result.code += sub_expr.code + result.reg + " = " + "min(" + result.reg + ',' + sub_expr.reg + ");\n";
        }
        return result;
    }
    Carrier operator()(Invert<Fields> expr) {
        auto result = visit(*this, *expr.a);//, b = visit(*this, *expr.b);
    	result.code += result.reg + " *= -1.f;\n";
        return result;
    }
    Carrier operator()(Move<Fields> expr) {
        auto prev_move = state.move;
        state.move += expr.v;
        auto result = visit(*this, *expr.a);
        state.move = prev_move;
        return result;
    }
    Carrier operator()(Rotate<Fields> expr) {
        auto prev_rotate = state.rotate;
        state.rotate *= expr.m;
        auto result = visit(*this, *expr.a);
        state.rotate = prev_rotate;
        return result;
    }

    Carrier assign_register(std::string code) {
        std::string reg = reg_name(state.next_reg++);
        return { "float " + reg + " = " + code, reg };
    }
    std::string reg_name(int id) {
        return "r" + std::to_string(id);
    }
    std::string move_rotate() {
        glm::vec3 m = state.rotate * state.move;
    	return (state.rotate == glm::mat3(1.f)? "" : float3x3(state.rotate) + "*")
    		+   'p'
    		+  (m == glm::vec3(0)             ? "" : " - " + float3(m));
    }
};

template<typename Fields>
std::string sdf(Expr<Fields> expr) {
    auto result = visit(CodeGen<Fields>{}, expr);
    return "float SDF(vec3 p)\n{\n" + result.code + "return " + result.reg + ";\n}\n";
}