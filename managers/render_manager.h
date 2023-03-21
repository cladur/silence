#ifndef SILENCE_RENDER_MANAGER_H
#define SILENCE_RENDER_MANAGER_H

#include <vector>

#include "display_manager.h"

#define VULKAN_HPP_NO_EXCEPTIONS
#include "vulkan/vulkan.hpp"

class RenderManager {
	// INITIALIZATION
	vk::Instance instance;
	vk::DebugUtilsMessengerEXT debug_messenger; // Vulkan debug output handle
	vk::PhysicalDevice chosen_gpu; // GPU chosen as the default device
	vk::Device device; // Vulkan device for commands
	vk::SurfaceKHR surface; // Vulkan window surface

	// SWAPCHAIN
	vk::SwapchainKHR swapchain; // from other articles
	// image format expected by the windowing system
	vk::Format swapchain_image_format;
	//array of images from the swapchain
	std::vector<vk::Image> swapchain_images;
	//array of image-views from the swapchain
	std::vector<vk::ImageView> swapchain_image_views;

	// COMMAND POOL
	vk::Queue graphics_queue; // queue we will submit to
	uint32_t graphics_queue_family; // family of that queue

	vk::CommandPool command_pool; //the command pool for our commands
	vk::CommandBuffer main_command_buffer; //the buffer we will record into

	// RENDERPASS
	vk::RenderPass render_pass;
	std::vector<vk::Framebuffer> framebuffers;

	void init_vulkan(DisplayManager &display_manager);
	void init_swapchain(DisplayManager &display_manager);
	void init_commands();

public:
	enum class Status {
		Ok,
		FailedToInitializeVulkan,
	};

	Status startup(DisplayManager &display_manager);
	void shutdown();
};

#endif //SILENCE_RENDER_MANAGER_H
