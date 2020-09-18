#pragma once

#include "expr.h"
#include "util.h"
#include "glm/glm.hpp"
#include <array>
#include <string>

template<typename Fields>
struct CodeGen {
	
    using Carrier = CodeCarrier;

    struct State {
    	std::string functions;
        int next_reg        = 0;
        int next_fun        = 0;
        glm::vec3 move      = glm::vec3(0);
        glm::mat3 rotate    = glm::mat3(1);
    } state;

    Carrier operator()(Box<Fields> expr) {
        return Var("float","box(" + move_rotate() + ", " + float3(expr.x, expr.y, expr.z) + ")");
    }
    Carrier operator()(Sphere<Fields> expr) {
        return Var("float","sphere(" + move_rotate() + ", " + float1(expr.r) + ")");
    }
    Carrier operator()(Cylinder<Fields> expr) {
        using namespace std::string_literals;
        return Var("float","cylinder"s + (char)expr.dir + "(" + move_rotate() + ", " +
            (expr.y == std::numeric_limits<float>::infinity() ? float1(expr.x) : float2(expr.x, expr.y)) + ")");
    }
    Carrier operator()(PlaneXZ<Fields> expr) {
        return Var("float","planeXZ(" + move_rotate() + ")");
    }
    Carrier operator()(Offset<Fields> expr) {
        auto sub_expr = visit(*this, *expr.a);
        sub_expr += sub_expr.reg + " -= " + float1(expr.r) + ";//offset";
        return sub_expr;
    }
	Carrier operator()(Invert<Fields> expr) {
        auto result = visit(*this, *expr.a);//, b = visit(*this, *expr.b);
    	result += result.reg + " *= -1.f; //invert";
        return result;
    }
    Carrier operator()(Intersect<Fields> expr) {
        auto result = visit(*this, *expr.a[0]);
        for (int i = 1; i < (int)expr.a.size(); ++i) {
            auto sub_expr = visit(*this, *expr.a[i]);
        	result.code += sub_expr.code;
            result += result.reg + " = " + "max(" + result.reg + ',' + sub_expr.reg + "); //intersect " + std::to_string(i);
        }
        return result;
    }
	Carrier operator()(Union<Fields> expr) {
        auto result = visit(*this, *expr.a[0]);
        for (int i = 1; i < (int)expr.a.size(); ++i) {
            auto sub_expr = visit(*this, *expr.a[i]);
        	result.code += sub_expr.code;
            result += result.reg + " = " + "min(" + result.reg + ',' + sub_expr.reg + "); //union " + std::to_string(i);
        }
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
    std::string move_rotate() {
        glm::vec3 m = state.rotate * state.move;
    	return (state.rotate == glm::mat3(1.f)? "" : float3x3(state.rotate) + "*")
    		+   'p'
    		+  (m == glm::vec3(0)             ? "" : " - " + float3(m));
    }

    // Defines a new function based on the code within code carrier and returns the name of the function.
	std::string Fun(const Carrier &carr, const std::string &arguments = "vec3 p")
    {
    	//state.next_reg = 0;
    	std::string name = "f" + std::to_string(state.next_fun++);
    	state.functions += + "\n" + carr.reg_type + " " + name + "(" + arguments + "){\n"
    						+ carr.code
    						+"\treturn " + carr.reg
    					+ ";\n}\n";
	    return name;
    }

	// Initializes a new variable of type type_ with value_ and adds the line into existing code carrier carr_.
	std::string Var(const std::string& type_, const std::string &value_, Carrier &carr_)
    {
        std::string name = "r" + std::to_string(state.next_reg);
    	++state.next_reg;
    	carr_.code += "\t" + type_ + " " + name + " = " + value_ + ";\n"  ;
    	return name;
    }

	// Initializes a new variable of type type_ with value_, and returns the corresponding code carrier.
	Carrier Var(const std::string& type_, const std::string &value_)
    {
    	Carrier carr = {"","",type_};
    	carr.reg = Var(type_, value_, carr);
    	return carr;
    }
	
};

template<typename Fields>
std::string sdf(Expr<Fields> expr) {
    auto result = visit(CodeGen<Fields>{}, expr);
    return "float SDF(vec3 p)\n{\n" + result.code + "return " + result.reg + ";\n}\n";
}