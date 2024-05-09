#pragma once

#include <string>
#include <filesystem>
#include <variant>

namespace fs = std::filesystem;

namespace ash
{
enum class FileError
{
    FILE_NOT_EXISTS,
    FILE_CAN_NOT_OPEN,
    FILE_UNKNOWN_ERROR
};

extern std::variant<std::string, FileError> read_text_file(const fs::path& file_path);

extern std::variant<std::string, FileError> read_shader(const fs::path& relative_file_path);
}
