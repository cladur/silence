#ifndef SILENCE_RENDER_MANAGER_H
#define SILENCE_RENDER_MANAGER_H

#include "core/camera/camera.h"

#include "magic_enum.hpp"

#include <glm/glm.hpp>

#include "ft2build.h"
#include FT_FREETYPE_H

#include "managers/display/display_manager.h"

#define VULKAN_HPP_NO_EXCEPTIONS
#include "vulkan/vulkan.hpp"

#include "vulkan-memory-allocator-hpp/vk_mem_alloc.hpp"

#include "render/vk_mesh.h"

#define VK_CHECK(x)                                                                                                    \
	do {                                                                                                               \
		vk::Result err = x;                                                                                            \
		if (err != vk::Result::eSuccess) {                                                                             \
			SPDLOG_ERROR("Detected Vulkan error: ({}) {}", magic_enum::enum_integer(err), magic_enum::enum_name(err)); \
			abort();                                                                                                   \
		}                                                                                                              \
	} while (false)

constexpr unsigned int FRAME_OVERLAP = 2;

struct MeshPushConstants {
	glm::vec4 data;
	glm::mat4 render_matrix;
};

struct UploadContext {
	vk::Fence upload_fence;
	vk::CommandPool command_pool;
	vk::CommandBuffer command_buffer;
};

struct DeletionQueue {
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()> &&function) {
		deletors.push_back(function);
	}

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) { // NOLINT(modernize-loop-convert)
			(*it)(); //call the function
		}

		deletors.clear();
	}
};

struct Texture {
	AllocatedImage image;
	vk::ImageView image_view;
};

struct Material {
	vk::DescriptorSet texture_set{ VK_NULL_HANDLE }; //texture defaulted to null
	vk::Pipeline pipeline;
	vk::PipelineLayout pipeline_layout;
};

struct RenderObject {
	Mesh *mesh;
	Material *material;
	glm::mat4 transform_matrix;
};

struct MeshInstance {
	Mesh *mesh;
	Material *material;
};

struct GPUCameraData {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
};

struct GPUSceneData {
	glm::vec4 fog_color; // w is for exponent
	glm::vec4 fog_distances; //x for min, y for max, zw unused.
	glm::vec4 ambient_color;
	glm::vec4 sunlight_direction; //w for sun power
	glm::vec4 sunlight_color;
};

struct GPUObjectData {
	glm::mat4 model_matrix;
};

struct FrameData {
	vk::Semaphore present_semaphore, render_semaphore;
	vk::Fence render_fence;

	vk::CommandPool command_pool; //the command pool for our commands
	vk::CommandBuffer main_command_buffer; //the buffer we will record into

	//buffer that holds a single GPUCameraData to use when render
	AllocatedBuffer camera_buffer;
	vk::DescriptorSet global_descriptor;

	AllocatedBuffer object_buffer;
	vk::DescriptorSet object_descriptor;
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

	vk::PipelineLayout ui_pipeline_layout;
	vk::Pipeline ui_pipeline;

	// MESHES
	Mesh triangle_mesh;
	Mesh box_mesh;
	Mesh plane_mesh;

	// DEPTH
	vk::ImageView depth_image_view;
	AllocatedImage depth_image;
	vk::Format depth_format;

	// DESCRIPTORS
	vk::DescriptorSetLayout global_set_layout;
	vk::DescriptorSetLayout object_set_layout;
	vk::DescriptorSetLayout single_texture_set_layout;
	vk::DescriptorPool descriptor_pool;

	vk::PhysicalDeviceProperties gpu_properties;

	UploadContext upload_context;

	GPUSceneData scene_parameters;
	AllocatedBuffer scene_parameter_buffer;

	vk::Extent2D window_extent;

	void init_vulkan(DisplayManager &display_manager);
	void init_swapchain(DisplayManager &display_manager);
	void init_commands();
	void init_default_renderpass();
	void init_framebuffers();
	void init_sync_structures();
	void init_descriptors();
	void init_pipelines();
	void init_scene();
	void init_imgui(GLFWwindow *window);

	//loads a shader module from a spir-v file. Returns false if it errors
	bool load_shader_module(const char *file_path, vk::ShaderModule *out_shader_module);

	void load_meshes();
	void upload_mesh(Mesh &mesh);

	size_t pad_uniform_buffer_size(size_t original_size) const;

public:
	enum class Status {
		Ok,
		FailedToInitializeVulkan,
	};

	// Frame storage
	FrameData frames[2];
	// TODO: get frame_number from some global getter instead of storing it here
	unsigned int frame_number = 0;

	vma::Allocator allocator;
	DeletionQueue main_deletion_queue;

	//default array of renderable objects
	std::vector<RenderObject> renderables;
	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, Mesh> meshes;

	//i hate the way of doing this but i couln;t figure out anything else 💀
	std::vector<Texture> glyphs;
	vk::Sampler glyph_sampler;

	// texture stuff
	std::unordered_map<std::string, Texture> loaded_textures;

	Status startup(DisplayManager &display_manager);
	void shutdown();

	//getter for the frame we are render to right now.
	FrameData &get_current_frame();

	AllocatedBuffer create_buffer(size_t alloc_size, vk::BufferUsageFlags usage, vma::MemoryUsage memory_usage) const;
	void immediate_submit(std::function<void(vk::CommandBuffer cmd)> &&function);

	//create material and add it to the map
	Material *create_material(vk::Pipeline pipeline, vk::PipelineLayout layout, const std::string &name);
	//returns nullptr if it can't be found
	Material *get_material(const std::string &name);
	//returns nullptr if it can't be found
	Mesh *get_mesh(const std::string &name);

	void load_images();

	void draw(Camera &camera);
	void draw_objects(Camera &camera, vk::CommandBuffer cmd, RenderObject *first, int count);

	Texture get_character_texture(FT_Face &face);

	vk::Sampler create_sampler(vk::Filter filter, vk::SamplerAddressMode address_mode);

	void update_descriptor_set_with_texture(Texture texture, vk::Sampler &sampler, Material &mat);
};

#endif //SILENCE_RENDER_MANAGER_H
