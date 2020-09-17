#pragma once

#include "expr.h"
#include "util.h"
#include "glm/glm.hpp"
#include <array>
#include <string>

#include "Dragonfly/config.h"


template<typename Fields>
struct Footmap {
	
    using Carrier = CodeCarrier;

    struct State {
    	std::string functions;
        int next_reg        = 0;
        int next_fun        = 0;
        glm::vec3 move      = glm::vec3(0);
        glm::mat3 rotate    = glm::mat3(1);
    } state;
	
    Carrier operator()(Box<Fields> expr) {
        return Var("vec4", "fm_box(" + move_rotate() + ", " + float3(expr.x, expr.y, expr.z) + ")");
    }
    
    Carrier operator()(Sphere<Fields> expr) {
        return Var("vec4", "fm_sphere(" + move_rotate() + ", " + float1(expr.r) + ")");
    }
    Carrier operator()(Cylinder<Fields> expr) {
        using namespace std::string_literals;
        return Var("vec4", "fm_cylinder"s + (char)expr.dir + "(" + move_rotate() + ", " +
            (expr.y == std::numeric_limits<float>::infinity() ? float1(expr.x) : float2(expr.x, expr.y)) + ")");
    }
    Carrier operator()(PlaneXZ<Fields> expr) {
        return Var("vec4", "fm_planeY(" + move_rotate() + ")");
    }

	Carrier operator()(Invert<Fields> expr) {
        auto ret = visit(*this, *expr.a);
    	ret += ret.reg + ".w *= -1.f;";
    	return ret;
    }
	
    Carrier operator()(Offset<Fields> expr) {
        Carrier sub = visit(*this, *expr.a);
    	sub += sub.reg + " -= " + float1(expr.r) +" * vec4(normalize(" + sub.reg + ".xyz),1); //offset";
        return sub;
    }
    Carrier operator()(Intersect<Fields> expr)
	{
    	ASSERT(expr.a.size()==2, "Too many argumets at Intersect CSG OP."); //todo more
    	int prev_regnumber = state.next_reg;
    	state.next_reg = 0;
    	std::string f = Fun(visit(*this, *expr.a[0]));  // Generates a new function from levels below...
    	state.next_reg = 0;
    	std::string g = Fun(visit(*this, *expr.a[1]));  // ... which is then stored and given a name.
    	state.next_reg = prev_regnumber;

        Carrier carr = Var("vec4", "vec4(0)"); // New variable initialized code carrier (variable is its register)
    	const std::string &reg = carr.reg;
    	
    	std::string fp  = Var("vec4", f + "(p)", carr); // New variable into existing code
    	std::string gp  = Var("vec4", g + "(p)", carr);
    	std::string gfp = Var("vec4", g + "(p + " + fp + ".xyz)", carr);
    	std::string fgp = Var("vec4", f + "(p + " + gp + ".xyz)", carr);
    	std::string mxp = Var("vec4", fp  + ".w > " + gp + ".w ? " + fp + " : " + gp, carr);
    	
        carr += reg + ".xyz = " + mxp + ".w<=0. ? "  + mxp + ".xyz : " + reg + ".xyz;"; // Carrier += string operator
        carr += reg + ".xyz = " + mxp + ".w>0.0 && " + gfp + ".w<0.001 ? " + fp + ".xyz" + " : " + reg + ".xyz;";
        carr += reg + ".xyz = " + mxp + ".w>0.0 && " + fgp + ".w<0.001 ? " + gp + ".xyz" + " : " + reg + ".xyz;";
    	
    	std::string vec = Var("vec3", "0.5 * ("+fp + ".xyz + " + gp + ".xyz + " + fgp + ".xyz + " + gfp + ".xyz)", carr);
    	
    	carr += reg + ".xyz = " + mxp + ".w>0.0 && " + fgp + ".w>=0.001 && " + gfp + ".w>=0.001 ? "
    							+ vec + " : " + reg + ".xyz;";
    	carr += reg + ".w = (" + mxp + ".w<0.?-1.:1.) * length(" + reg + ".xyz);";
		return carr;
    }
	Carrier operator()(Union<Fields> expr)
	{
        Carrier ret = visit(*this, *expr.a[0]);
        for (auto i = 1; i < expr.a.size(); ++i) {
        	Carrier sub = visit(*this, *expr.a[i]);
        	ret.code += sub.code;
        	ret += ret.reg + " = " + ret.reg +".w<" + sub.reg + ".w ? " + ret.reg + " : " + sub.reg + "; //union_" + std::to_string(i);
        }
    	ret += ret.reg + ".w = (" + ret.reg + ".w > 0. ? 1. : -1.) * length(" + ret.reg + ".xyz); //union end";
        return ret;
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
std::string footmap(Expr<Fields> expr) {
	auto visitor = Footmap<Fields>{};
    auto result = visit(visitor, expr);
    return visitor.state.functions + "\nvec4 footmap(vec3 p)\n{\n" + result.code + "\treturn " + result.reg + ";\n}\n"
	+ "float SDF(vec3 p)\n{\n\tvec4 m = footmap(p);\n\treturn length(m.xyz)*(m.w>0.?1.:-1.);\n}\n";
}