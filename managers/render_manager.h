#ifndef SILENCE_RENDER_MANAGER_H
#define SILENCE_RENDER_MANAGER_H

#include <deque>
#include <functional>
#include <ranges>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "display_manager.h"

#define VULKAN_HPP_NO_EXCEPTIONS
#include "vulkan/vulkan.hpp"

#include "vulkan-memory-allocator-hpp/vk_mem_alloc.hpp"

#include "rendering/vk_mesh.h"

constexpr unsigned int FRAME_OVERLAP = 2;

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
		for (auto &deletor : std::ranges::reverse_view(deletors)) {
			deletor(); //call the function
		}

		deletors.clear();
	}
};

struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject {
	Mesh *mesh;
	Material *material;
	glm::mat4 transformMatrix;
};

struct FrameData {
	vk::Semaphore present_semaphore, render_semaphore;
	vk::Fence render_fence;

	vk::CommandPool command_pool; //the command pool for our commands
	vk::CommandBuffer main_command_buffer; //the buffer we will record into
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

	// GRAPHICS QUEUE
	vk::Queue graphics_queue; // queue we will submit to
	uint32_t graphics_queue_family; // family of that queue

	// RENDERPASS
	vk::RenderPass render_pass;
	std::vector<vk::Framebuffer> framebuffers;

	// PIPELINE
	vk::PipelineLayout mesh_pipeline_layout;
	vk::Pipeline mesh_pipeline;

	// MESHES
	Mesh triangle_mesh;
	Mesh monkey_mesh;

	// DEPTH
	vk::ImageView depth_image_view;
	AllocatedImage depth_image;
	vk::Format depth_format;

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
	void init_scene();
	void init_imgui(GLFWwindow *window);

	//loads a shader module from a spir-v file. Returns false if it errors
	bool load_shader_module(const char *file_path, vk::ShaderModule *out_shader_module);

	void load_meshes();
	void upload_mesh(Mesh &mesh);

public:
	enum class Status {
		Ok,
		FailedToInitializeVulkan,
	};

	// Frame storage
	FrameData frames[2];
	// TODO: get frame_number from some global getter instead of storing it here
	unsigned int frame_number = 0;

	//default array of renderable objects
	std::vector<RenderObject> renderables;
	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, Mesh> meshes;

	Status startup(DisplayManager &display_manager);
	void shutdown();

	//getter for the frame we are rendering to right now.
	FrameData &get_current_frame();

	//create material and add it to the map
	Material *create_material(vk::Pipeline pipeline, vk::PipelineLayout layout, const std::string &name);
	//returns nullptr if it can't be found
	Material *get_material(const std::string &name);
	//returns nullptr if it can't be found
	Mesh *get_mesh(const std::string &name);

	void draw();
	void draw_objects(vk::CommandBuffer cmd, RenderObject *first, int count) const;
};

#endif //SILENCE_RENDER_MANAGER_H
