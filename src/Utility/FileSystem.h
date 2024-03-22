#pragma once
// std include
#include <string>
#include <filesystem>
// thirdparty include

// Aurora include


namespace Aurora
{

class FileSystem
{
public:
    static std::string GetFullPath(const std::string& relative_path)
    {
#ifdef PROJECT_ROOT_DIR
        std::filesystem::path project_root_dir(PROJECT_ROOT_DIR);
        std::filesystem::path relative_path_fs(relative_path);
        return std::filesystem::absolute(project_root_dir / relative_path_fs).string();
#else
        return std::filesystem::absolute(relative_path).string();
#endif
    }

    static std::string GetFileName(const std::string& file_path)
    {
        std::filesystem::path file_path_fs(file_path);
        return file_path_fs.filename().string();
    }
};
} // namespace Aurora