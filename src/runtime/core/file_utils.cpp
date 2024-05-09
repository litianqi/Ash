#include "file_utils.h"
#include <regex>
#include <fstream>
#include <algorithm>
#include <vector>
#include "app/app.h"

namespace ash
{
std::variant<std::string, FileError> read_text_file(const fs::path& file_path)
{
    if (!fs::exists(file_path))
    {
        return FileError::FILE_NOT_EXISTS;
    }
    try
    {
        std::ifstream file;
        file.open(file_path, std::ios::in);
        if (!file.is_open())
        {
            return FileError::FILE_CAN_NOT_OPEN;
        }
        return std::string{(std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>())};
    }
    catch (...)
    {
        return FileError::FILE_UNKNOWN_ERROR;
    }
}

std::variant<std::string, FileError> read_shader(const fs::path& relative_file_path)
{
    auto file_path = BaseApp::get()->get_shaders_dir() / relative_file_path;
    auto result = read_text_file(file_path);
    if (auto* file = std::get_if<std::string>(&result))
    {
        std::regex rgx("#include\\s+\"(.+)\"");
        std::smatch matches;

        while (std::regex_search(*file, matches, rgx))
        {
            assert(matches.size() == 2);
            auto& match = matches[1];
            auto include_result = read_shader(match.str());
            if (auto* include_file = std::get_if<std::string>(&include_result))
            {
                file->replace(matches.position(), matches.length(), *include_file);
            }
            else
            {
                spdlog::error("Failed to find include file `{}`", match.str());
                return include_result;
            }
        }
    }
    return result;
}
} // namespace ash
