#ifndef SILENCE_DISPLAY_MANAGER_H
#define SILENCE_DISPLAY_MANAGER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <utility>

class DisplayManager {
public:
	enum class Status {
		Ok,
		FailedToInitializeGlfw,
		FailedToCreateWindow,
		VulkanNotSupported,
	};

	GLFWwindow *window;

	Status startup();
	void shutdown();

	VkSurfaceKHR create_surface(VkInstance &instance) const;

	std::pair<int, int> get_window_size() const;
	void poll_events() const;
	bool window_should_close() const;
};

#endif //SILENCE_DISPLAY_MANAGER_H
