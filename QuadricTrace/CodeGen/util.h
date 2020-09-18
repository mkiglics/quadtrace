#pragma once

#include "glm/glm.hpp"
#include <string>

std::string float1(float x);
std::string float2(float x, float y);
std::string float2(glm::vec2 v);
std::string float3(float x, float y, float z);
std::string float3(glm::vec3 v);
std::string float3x3(glm::mat3 m);

glm::mat3 rotateX(float angle);
glm::mat3 rotateY(float angle);
glm::mat3 rotateZ(float angle);

float length(float x, float y);
float length(float x, float y, float z);

struct CodeCarrier
{
    	std::string code, reg, reg_type;
};

inline CodeCarrier& operator +=(CodeCarrier& carr_, const std::string &statement_) {
	carr_.code += "\t" + statement_ + "\n";
	return carr_;
}
