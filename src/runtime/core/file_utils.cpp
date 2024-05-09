#include "file_utils.h"
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
    return read_text_file(file_path);
}
} // namespace ash
