#define GLM_FORCE_SWIZZLE
#define _USE_MATH_DEFINES
#include "../bounding_opt.h"
#include "../codegen.h"
#include "glm/glm.hpp"
#include <cmath>
#include <fstream>
#include "models.h"

#include <iostream>

using namespace std;
using namespace glm;

MyExpr* model10_expr(float r) {

    MyExpr* out = nullptr;
    
    MyExpr* b = box<Fields>(0.5, 0.5, 0.5);
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            for (int k = 0; k < 8; ++k)
            {
                if (i * j * k % 7 > 0) continue;
                MyExpr* o = sphere(1/13.f, Fields{ 1 });
                o = move(glm::vec3((i - 4) / 8.0 + 1/16.f, (j - 4) / 8.0 + 1 / 16.f, (k - 4) / 8.0 + 1 / 16.f), o);
                b = subtract<Fields>(b, o);
            }
        }

    }

    out = b;

    return out;
}
