// std include
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <memory>
#include <fstream>
#include <sstream>
#include <unordered_set>
// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include <shaderc/shaderc.hpp>
// Aurora include
#include "glWrapper/Shader.h"

namespace Aurora
{

namespace
{
struct ShadercIncludeResult
{
    shaderc_include_result result{};
    std::string source_name;
    std::string content;
    std::string full_path;
    bool remove_from_stack = false;

    ShadercIncludeResult(std::string source_name_in,
                         std::string content_in,
                         std::string full_path_in,
                         bool remove_from_stack_in)
        : source_name(std::move(source_name_in)),
          content(std::move(content_in)),
          full_path(std::move(full_path_in)),
          remove_from_stack(remove_from_stack_in)
    {
        result.source_name = source_name.c_str();
        result.source_name_length = source_name.size();
        result.content = content.c_str();
        result.content_length = content.size();
        result.user_data = this;
    }
};

static bool ReadTextFile(const std::filesystem::path& path, std::string* out)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    *out = buffer.str();
    return true;
}

static std::string NormalizePath(const std::filesystem::path& path)
{
    std::error_code ec;
    auto absolute_path = std::filesystem::absolute(path, ec);
    if (ec)
    {
        return path.lexically_normal().string();
    }
    return absolute_path.lexically_normal().string();
}

class ShadercIncluder final : public shaderc::CompileOptions::IncluderInterface
{
public:
    explicit ShadercIncluder(std::vector<std::filesystem::path> include_dirs)
        : m_include_dirs(std::move(include_dirs)) {}

    shaderc_include_result* GetInclude(const char* requested_source,
                                       shaderc_include_type type,
                                       const char* requesting_source,
                                       size_t include_depth) override
    {
        (void)include_depth;
        if (requested_source == nullptr)
        {
            return MakeErrorResult("Include path is null");
        }

        std::filesystem::path requested_path(requested_source);
        if (requested_path.is_absolute())
        {
            if (auto* result = TryResolveCandidate(requested_path))
            {
                return result;
            }
            return MakeErrorResult("Include file not found: " + requested_path.string());
        }

        std::vector<std::filesystem::path> search_dirs;
        if (type == shaderc_include_type_relative)
        {
            if (requesting_source != nullptr && requesting_source[0] != '\0')
            {
                std::filesystem::path requesting_path(requesting_source);
                auto requesting_dir = requesting_path.parent_path();
                if (!requesting_dir.empty())
                {
                    search_dirs.push_back(requesting_dir);
                }
            }
        }
        for (const auto& dir : m_include_dirs)
        {
            search_dirs.push_back(dir);
        }

        for (const auto& dir : search_dirs)
        {
            auto candidate = dir / requested_path;
            if (auto* result = TryResolveCandidate(candidate))
            {
                return result;
            }
        }

        return MakeErrorResult("Include file not found: " + std::string(requested_source));
    }

    void ReleaseInclude(shaderc_include_result* data) override
    {
        if (data == nullptr || data->user_data == nullptr)
        {
            return;
        }
        auto* include = static_cast<ShadercIncludeResult*>(data->user_data);
        if (include->remove_from_stack)
        {
            m_in_stack.erase(include->full_path);
        }

        auto it = std::find_if(m_live_includes.begin(), m_live_includes.end(),
                               [include](const std::unique_ptr<ShadercIncludeResult>& entry)
                               {
                                   return entry.get() == include;
                               });
        if (it != m_live_includes.end())
        {
            m_live_includes.erase(it);
        }
    }

private:
    shaderc_include_result* TrackResult(std::unique_ptr<ShadercIncludeResult> include)
    {
        auto* result = &include->result;
        m_live_includes.emplace_back(std::move(include));
        return result;
    }

    shaderc_include_result* TryResolveCandidate(const std::filesystem::path& path)
    {
        std::error_code ec;
        if (!std::filesystem::exists(path, ec) || !std::filesystem::is_regular_file(path, ec))
        {
            return nullptr;
        }

        std::string full_path = NormalizePath(path);
        if (m_in_stack.count(full_path) != 0)
        {
            return MakeErrorResult("Include cycle detected: " + full_path);
        }

        std::string content;
        if (!ReadTextFile(path, &content))
        {
            return nullptr;
        }

        const bool has_pragma_once = (content.find("#pragma once") != std::string::npos);
        if (m_once_files.count(full_path) != 0)
        {
            return MakeResult(full_path, std::string(), false, full_path);
        }
        if (has_pragma_once)
        {
            m_once_files.insert(full_path);
        }

        m_in_stack.insert(full_path);
        return MakeResult(full_path, std::move(content), true, full_path);
    }

    shaderc_include_result* MakeResult(std::string source_name,
                                       std::string content,
                                       bool remove_from_stack,
                                       std::string full_path)
    {
        auto include = std::make_unique<ShadercIncludeResult>(std::move(source_name),
                                                              std::move(content),
                                                              std::move(full_path),
                                                              remove_from_stack);
        return TrackResult(std::move(include));
    }

    shaderc_include_result* MakeErrorResult(const std::string& message)
    {
        std::string content = std::string("#error ") + message + "\n";
        return MakeResult("<include_error>", std::move(content), false, std::string());
    }

    std::vector<std::filesystem::path> m_include_dirs;
    std::unordered_set<std::string> m_once_files;
    std::unordered_set<std::string> m_in_stack;
    std::vector<std::unique_ptr<ShadercIncludeResult>> m_live_includes;
};

static std::vector<std::filesystem::path> BuildDefaultIncludeDirs(const std::string& source_path)
{
    std::vector<std::filesystem::path> dirs;
    if (!source_path.empty())
    {
        std::filesystem::path src_path(source_path);
        auto parent = src_path.parent_path();
        if (!parent.empty())
        {
            dirs.push_back(parent);
        }
    }

#ifdef PROJECT_ROOT_DIR
    std::filesystem::path project_root(PROJECT_ROOT_DIR);
    dirs.push_back(project_root / "shaders");
    dirs.push_back(project_root / "shaders" / "include");
#endif

    std::vector<std::filesystem::path> unique;
    std::unordered_set<std::string> seen;
    for (const auto& dir : dirs)
    {
        if (dir.empty())
        {
            continue;
        }
        std::error_code ec;
        if (!std::filesystem::exists(dir, ec))
        {
            continue;
        }
        auto normalized = NormalizePath(dir);
        if (seen.insert(normalized).second)
        {
            unique.push_back(dir);
        }
    }
    return unique;
}

static bool PreprocessShaderWithShaderc(const std::string& source,
                                        const std::string& source_path,
                                        std::string* out_preprocessed)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetSourceLanguage(shaderc_source_language_glsl);
    options.SetIncluder(std::make_unique<ShadercIncluder>(BuildDefaultIncludeDirs(source_path)));

    auto result = compiler.PreprocessGlsl(source,
                                          shaderc_glsl_infer_from_source,
                                          source_path.c_str(),
                                          options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        spdlog::error("Shader preprocess failed: {}. {}", source_path, result.GetErrorMessage());
        return false;
    }

    out_preprocessed->assign(result.cbegin(), result.cend());
    return true;
}
} // namespace

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

    if (!shader_defines.empty())
    {
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
    }

    std::string preprocessed_shader;
    if (!PreprocessShaderWithShaderc(shader_code, shader_path, &preprocessed_shader))
    {
        spdlog::error("Failed to preprocess shader: {}", shader_path);
        return false;
    }
    shader_code = std::move(preprocessed_shader);

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
