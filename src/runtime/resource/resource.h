#pragma once

#include <filesystem>

namespace fs = std::filesystem;

namespace ash
{
class Resource
{
  public:
    Resource() = default;
    virtual ~Resource() = default;
    
    std::string name;
};

template <typename T = Resource>
using ResourcePtr = std::shared_ptr<T>;

template <typename T = Resource>
using ResourceWeakPtr = std::weak_ptr<T>;

template <typename T = Resource, typename... Args>
constexpr ResourcePtr<T> create_resource(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
} // namespace ash
