#ifndef SILENCE_DISPLAY_MANAGER_H
#define SILENCE_DISPLAY_MANAGER_H

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class DisplayManager {
public:
	enum class Status {
		Ok,
		FailedToInitializeGlfw,
		FailedToCreateWindow,
		VulkanNotSupported,
	};

	GLFWwindow *window;

	bool is_window_resizable;
	int default_window_pos[2] = { 64, 64 };

	static DisplayManager &get();

	Status startup(const std::string &window_name, bool resizable = false);
	void shutdown();

	void toggle_fullscreen() const;
	void set_windowed_full_hd() const;

	void capture_mouse(bool capture) const;

	[[nodiscard]] int get_refresh_rate() const;

	[[nodiscard]] bool was_window_resized() const;

	[[nodiscard]] glm::vec2 get_framebuffer_size() const;
	[[nodiscard]] glm::vec2 get_window_size() const;
	void poll_events();
	[[nodiscard]] bool window_should_close() const;
};

#endif //SILENCE_DISPLAY_MANAGER_H
