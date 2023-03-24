#ifndef SILENCE_RENDER_MANAGER_H
#define SILENCE_RENDER_MANAGER_H

#include <deque>
#include <functional>
#include <vector>

#include <glm/glm.hpp>

#include "display_manager.h"

#define VULKAN_HPP_NO_EXCEPTIONS
#include "vulkan/vulkan.hpp"

#include "vulkan-memory-allocator-hpp/vk_mem_alloc.hpp"

#include "rendering/vk_mesh.h"

struct MeshPushConstants {
	glm::vec4 data;
	glm::mat4 render_matrix;
};

struct DeletionQueue {
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()> &&function) {
		deletors.push_back(function);
	}

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); //call the function
		}

		deletors.clear();
	}
};

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

	// PIPELINE
	vk::PipelineLayout triangle_pipeline_layout;
	vk::PipelineLayout mesh_pipeline_layout;
	vk::Pipeline triangle_pipeline;
	vk::Pipeline red_triangle_pipeline;
	vk::Pipeline mesh_pipeline;

	Mesh triangle_mesh;
	Mesh monkey_mesh;

	vma::Allocator allocator;

	DeletionQueue main_deletion_queue;

	vk::Extent2D window_extent;

	void init_vulkan(DisplayManager &display_manager);
	void init_swapchain(DisplayManager &display_manager);
	void init_commands();
	void init_default_renderpass();
	void init_framebuffers();
	void init_sync_structures();
	void init_pipelines();

	//loads a shader module from a spir-v file. Returns false if it errors
	bool load_shader_module(const char *file_path, vk::ShaderModule *out_shader_module);

	void load_meshes();
	void upload_mesh(Mesh &mesh);

public:
	vk::Semaphore present_semaphore, render_semaphore;
	vk::Fence render_fence;

	enum class Status {
		Ok,
		FailedToInitializeVulkan,
	};

	Status startup(DisplayManager &display_manager);
	void shutdown();

	void draw();
};

#endif //SILENCE_RENDER_MANAGER_H
