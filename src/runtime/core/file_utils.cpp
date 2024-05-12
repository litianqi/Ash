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
        // 1. open the file:
        std::ifstream file(file_path);

        // 2. get its size:
        file.seekg(0, std::ios::end);
        auto file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        // 3. reserve capacity:
        std::string buffer(file_size, ' ');

        // 4. read the data:
        file.read(&buffer[0], file_size);
        return buffer;
    }
    catch (...)
    {
        return FileError::UNKNOWN_ERROR;
    }
}

std::variant<std::string, FileError> read_shader(const fs::path& relative_file_path)
{
    auto file_path =
        relative_file_path.is_relative() ? BaseApp::get()->get_shaders_dir() / relative_file_path : relative_file_path;
    auto result = read_text_file(file_path);
    if (auto* file = std::get_if<std::string>(&result))
    {
        std::regex rgx("#include\\s+\"(.+)\"");
        std::smatch matches;

        while (std::regex_search(*file, matches, rgx))
        {
            assert(matches.size() == 2);
            auto& match = matches[1];
            auto include_result = read_shader(file_path.parent_path() / match.str());
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

std::variant<std::vector<uint8_t>, FileError> read_binary_file(const fs::path& file_path)
{
    if (!fs::exists(file_path))
    {
        return FileError::FILE_NOT_EXISTS;
    }
    try
    {
        // 1. open the file:
        std::ifstream file(file_path, std::ios::binary);
        file.unsetf(std::ios::skipws); // Stop eating new lines in binary mode!!!

        // 2. get its size:
        file.seekg(0, std::ios::end);
        auto file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        // 3. reserve capacity:
        std::vector<uint8_t> buffer;
        buffer.resize(file_size);

        // 4. read the data:
        file.read(reinterpret_cast<std::ifstream::char_type*>(buffer.data()), file_size);
        return buffer;
    }
    catch (...)
    {
        return FileError::UNKNOWN_ERROR;
    }
}
} // namespace ash
