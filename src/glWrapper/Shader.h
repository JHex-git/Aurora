#pragma once
#include <string>
#include <vector>

#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/opengl/glm/glm/glm.hpp"

namespace Aurora
{

enum class ShaderType : GLenum
{
    VertexShader = GL_VERTEX_SHADER,
    TessellationControlShader = GL_TESS_CONTROL_SHADER,
    TessellationEvaluationShader = GL_TESS_EVALUATION_SHADER,
    GeometryShader = GL_GEOMETRY_SHADER,
    FragmentShader = GL_FRAGMENT_SHADER
};

class Shader
{
public:
    Shader(ShaderType type);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other): m_shaderID(other.m_shaderID), m_type(other.m_type)
    {
        other.m_shaderID = 0;
    }
    Shader& operator=(Shader&& other)
    {
        if (this != &other)
        {
            m_shaderID = other.m_shaderID;
            m_type = other.m_type;
            other.m_shaderID = 0;
        }
        return *this;
    }

    // This should be called before Load()
    void SetFlag(std::string&& flag) { m_flags.emplace_back(std::move(flag)); }

    // This should be called before Load()
    template <typename T>
    void SetOption(std::string&& key, T&& value) { m_options.emplace(std::move(key), std::to_string(std::forward<T>(value))); }

    bool Load(const std::string& shader_path);

    GLuint GetShaderID() const { return m_shaderID; }
    ShaderType GetShaderType() const { return m_type; }

private:
    unsigned int m_shaderID;
    ShaderType m_type;

    std::vector<std::string> m_flags; // On/Off DEFINEs
    std::unordered_map<std::string, std::string> m_options; // Key-Value DEFINEs
};

class ShaderProgram
{
public:
    ShaderProgram();
    ~ShaderProgram();

    bool Load(const std::vector<Shader>& shaders);

    void Bind() const;
    void Unbind() const;

    void SetUniform(const std::string& name, int value) const;
    void SetUniform(const std::string& name, float value) const;
    void SetUniform(const std::string& name, const glm::vec2& value) const;
    void SetUniform(const std::string& name, const glm::vec3& value) const;
    void SetUniform(const std::string& name, const glm::vec4& value) const;
    void SetUniform(const std::string& name, const glm::mat4& value) const;

private:
    unsigned int m_programID;
};
} // namespace Aurora
