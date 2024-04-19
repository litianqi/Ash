#pragma once

namespace ash
{
class FPSCounter
{
  public:
    explicit FPSCounter(float interval = 0.5f) : interval(interval)
    {
    }

    void tick(float dt)
    {
        num_frames++;
        accumulated_time += dt;

        if (accumulated_time > interval)
        {
            current_fps = static_cast<float>(num_frames / accumulated_time);
            num_frames = 0;
            accumulated_time = 0;
        }
    }

    float get_fps() const
    {
        return current_fps;
    }

  private:
    const float interval = 0.5f;
    unsigned int num_frames = 0;
    double accumulated_time = 0;
    float current_fps = 0.0f;
};
} // namespace ash
