#ifndef SILENCE_DISPLAY_MANAGER_H
#define SILENCE_DISPLAY_MANAGER_H

#define GLFW_INCLUDE_VULKAN
#include "../core/input/input_devices.h"
#include "../core/input/input_key.h"
#include "../core/input/input_manager.h"
#include "../core/input/multiplatform_input.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
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

	//input
	MultiplatformInput m_input{};
	std::unordered_map<InputKey, InputDeviceState> get_gamepad_state(int index);

	Status startup();
	void shutdown();
	void setup_input();

	VkSurfaceKHR create_surface(VkInstance &instance) const;

	std::pair<int, int> get_window_size() const;
	void poll_events();
	bool window_should_close() const;
};

#endif //SILENCE_DISPLAY_MANAGER_H
