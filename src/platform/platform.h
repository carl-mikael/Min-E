#pragma once

struct GLFWwindow;

namespace platform
{
struct window_data
{
	int width, height;
	GLFWwindow *platform_window;
};

struct tick_timer
{
	double tick_time;
	double last_time;
};

bool init(int width, int height, window_data &out_window) noexcept;

inline constexpr float get_aspect(const window_data &window,
                                  const bool is_two_players) noexcept
{
	return (static_cast<float>(window.width) / (is_two_players ? 2 : 1)) /
	       static_cast<float>(window.height);
}

bool update(const platform::window_data &window,
            platform::tick_timer &timer) noexcept;

inline double delta_time(const tick_timer &timer) noexcept
{
	return timer.tick_time - timer.last_time;
}

} // namespace platform
