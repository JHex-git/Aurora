#pragma once
// std include

// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
#include "thirdparty/assimp/include/assimp/matrix4x4.h"
// Aurora include


namespace Aurora
{

class AssimpHelper
{
public:
    static glm::mat4 MatrixToGLM(const aiMatrix4x4& matrix)
    {
        glm::mat4 result;
        result[0][0] = matrix.a1; result[0][1] = matrix.b1; result[0][2] = matrix.c1; result[0][3] = matrix.d1;
        result[1][0] = matrix.a2; result[1][1] = matrix.b2; result[1][2] = matrix.c2; result[1][3] = matrix.d2;
        result[2][0] = matrix.a3; result[2][1] = matrix.b3; result[2][2] = matrix.c3; result[2][3] = matrix.d3;
        result[3][0] = matrix.a4; result[3][1] = matrix.b4; result[3][2] = matrix.c4; result[3][3] = matrix.d4;
        return result;
    }
};
} // namespace Aurora