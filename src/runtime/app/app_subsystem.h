#pragma once

namespace ash
{
class AppSubsystem
{
  public:
    virtual ~AppSubsystem() = default;
    AppSubsystem(const AppSubsystem&) = delete;
    void operator=(const AppSubsystem&) = delete;

  protected:
    AppSubsystem() = default;
    friend class BaseApp;
};
} // namespace ash
