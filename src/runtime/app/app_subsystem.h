#pragma once

namespace ash
{
class AppSubsystem
{
  public:
    AppSubsystem() = default;
    virtual ~AppSubsystem() = default;
    AppSubsystem(const AppSubsystem&) = delete;
    void operator=(const AppSubsystem&) = delete;
};
} // namespace ash
