#pragma once

#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
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

static inline std::variant<std::string, FileError> read_text_file(const fs::path& file_path)
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
}
