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
// ------------------  File utils  ------------------

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

// ------------------ Vector utils ------------------

template <typename T>
bool vector_contains(const std::vector<T>& vec, const T& value)
{
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}

template <typename T>
std::vector<T>::const_iterator vector_find(const std::vector<T>& vec, const T& value)
{
    return std::find(vec.begin(), vec.end(), value);
}

template <typename T>
void vector_push_back_unique(std::vector<T>& vec, const T& value)
{
    if (!vector_contains(vec, value))
    {
        vec.push_back(value);
    }
}

template <typename T>
void vector_erase_first(std::vector<T>& vec, const T& value)
{
    auto it = std::find(vec.begin(), vec.end(), value);
    if (it != vec.end())
    {
        vec.erase(it);
    }
}

template <typename T>
void vector_erase_all(std::vector<T>& vec, const T& value)
{
    vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end());
}
}
