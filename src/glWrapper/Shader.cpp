// std include
#include <fstream>
#include <sstream>
// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "glWrapper/Shader.h"

namespace Aurora
{

Shader::Shader(ShaderType type): m_type(type)
{
    m_shaderID = glCreateShader(static_cast<GLenum>(type));
}

Shader::~Shader()
{
    if (m_shaderID != 0) spdlog::info("Shader {} is deleted", m_shaderID);
    glDeleteShader(m_shaderID);
}

bool Shader::Load(const std::string& shader_path)
{
    std::ifstream shader_file(shader_path);
    if (!shader_file.is_open())
    {
        spdlog::error("Failed to open shader file: {}", shader_path);
        return false;
    }

    std::stringstream shader_stream;
    shader_stream << shader_file.rdbuf();
    shader_file.close();

    std::string shader_defines;
    for (const auto& flag : m_flags)
    {
        shader_defines += "#define " + flag + "\n";
    }
    for (const auto& option : m_options)
    {
        shader_defines += "#define " + option.first + " " + option.second + "\n";
    }

    std::string shader_code = shader_stream.str();

    // add shader defines after #version xxx\n
    auto version_pos = shader_code.find("#version");
    if (version_pos != std::string::npos)
    {
        auto pos = shader_code.find('\n', version_pos + strlen("#version"));
        shader_code.insert(pos + 1, shader_defines);
    }
    else
    {
        shader_code = shader_defines + shader_code;
    }

    const char* shader_code_ptr = shader_code.c_str();
    glShaderSource(m_shaderID, 1, &shader_code_ptr, nullptr);
    glCompileShader(m_shaderID);

    int success;
    char info_log[512];
    int length;
    glGetShaderiv(m_shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(m_shaderID, 512, &length, info_log);
        spdlog::error("Failed to compile shader: {}. {}", shader_path, info_log);
        return false;
    }

    spdlog::info("Shader {} loaded", shader_path);
    return true;
}

ShaderProgram::ShaderProgram()
{
    m_programID = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(m_programID);
}

bool ShaderProgram::Load(const std::vector<Shader>& shaders)
{
    for (const auto& shader : shaders)
    {
        glAttachShader(m_programID, shader.GetShaderID());
    }
    glLinkProgram(m_programID);

    int success = 1;
    char info_log[512];
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(m_programID, 512, nullptr, info_log);
        spdlog::error("Failed to link shader program. {}", info_log);
        return false;
    }

    for (const auto& shader : shaders)
    {
        glDetachShader(m_programID, shader.GetShaderID());
    }

    spdlog::info("Shader program {} loaded", m_programID);
    return true;
}

void ShaderProgram::Bind() const
{
    glUseProgram(m_programID);
}

void ShaderProgram::Unbind() const
{
    glUseProgram(0);
}

void ShaderProgram::SetUniform(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(m_programID, name.c_str()), value);
}

void ShaderProgram::SetUniform(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(m_programID, name.c_str()), value);
}

void ShaderProgram::SetUniform(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(m_programID, name.c_str()), 1, &value[0]);
}

void ShaderProgram::SetUniform(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(m_programID, name.c_str()), 1, &value[0]);
}

void ShaderProgram::SetUniform(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(m_programID, name.c_str()), 1, &value[0]);
}

void ShaderProgram::SetUniform(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_programID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}
} // namespace Aurora
