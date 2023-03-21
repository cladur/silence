#include "display_manager.h"

#include <assert.h>

DisplayManager::Status DisplayManager::startup()
{
	if (!glfwInit())
	{
		return Status::FailedToInitializeGlfw;
	}

	// No need to create context since we're using Vulkan, not OpenGL(ES).
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(640, 480, "Silence Game", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return Status::FailedToCreateWindow;
	}

	if (!glfwVulkanSupported())
	{
		glfwTerminate();
		return Status::VulkanNotSupported;
	}

	return Status::Ok;
}

void DisplayManager::shutdown()
{
}

VkSurfaceKHR DisplayManager::create_surface(VkInstance &instance) const
{
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(instance, window, NULL, &surface);
	if (err)
	{
		assert(false);
	}
	return surface;
}

void DisplayManager::poll_events() const
{
	glfwPollEvents();
}

bool DisplayManager::window_should_close() const
{
	assert(window != nullptr);
	return glfwWindowShouldClose(window);
}

std::pair<int, int> DisplayManager::get_window_size() const
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	return std::make_pair(width, height);
}
