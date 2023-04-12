#ifndef SILENCE_RENDER_MANAGER_H
#define SILENCE_RENDER_MANAGER_H

#include "core/camera/camera.h"

#include "managers/display/display_manager.h"
#include "material_system.h"
#include "render_system.h"
#include "vk_push_buffer.h"
#include "vk_types.h"
#include <vulkan/vulkan_handles.hpp>

#define VULKAN_HPP_NO_EXCEPTIONS
#include "vulkan/vulkan.hpp"

#include "vulkan-memory-allocator-hpp/vk_mem_alloc.hpp"

#include "vk_mesh.h"
#include "vk_scene.h"
#include "vk_shaders.h"

namespace vk_util {
class DescriptorLayoutCache;
class DescriptorAllocator;
class Material;

} //namespace vk_util

constexpr unsigned int FRAME_OVERLAP = 2;

struct CullParams {
	glm::mat4 viewmat;
	glm::mat4 projmat;
	bool occlusion_cull;
	bool frustrum_cull;
	float draw_dist;
	bool aabb;
	glm::vec3 aabb_min;
	glm::vec3 aabb_max;
};

struct /*alignas(16)*/ DrawCullData {
	glm::mat4 view_mat;
	float p00, p11, znear, zfar; // symmetric projection parameters
	float frustum[4]; // data for left/right/top/bottom frustum planes
	float lod_base, lod_step; // lod distance i = base * pow(step, i)
	float pyramid_width, pyramid_height; // depth pyramid size in texels

	uint32_t draw_count;

	int culling_enabled;
	int lod_enabled;
	int occlusion_enabled;
	int distance_check;
	int aabb_check;
	float aabb_min_x;
	float aabb_min_y;
	float aabb_min_z;
	float aabb_max_x;
	float aabb_max_y;
	float aabb_max_z;
};

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

struct MeshObject {
	Mesh *mesh = nullptr;
	vk_util::Material *material = nullptr;

	uint32_t custom_sort_key{ 0 };
	RenderBounds bounds{};

	glm::mat4 transform_matrix{};

	uint32_t b_draw_forward_pass : 1 = 0U;
	uint32_t b_draw_shadow_pass : 1 = 0U;
};

struct MeshInstance {
	Mesh *mesh;
	vk_util::Material *material;
	Handle<RenderObject> object_id;
	bool registered = false;
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
	glm::vec4 origin_rad; // bounds
	glm::vec4 extents; // bounds
};

struct FrameData {
	vk::Semaphore present_semaphore, render_semaphore;
	vk::Fence render_fence;

	DeletionQueue frame_deletion_queue;

	vk::CommandPool command_pool; //the command pool for our commands
	vk::CommandBuffer main_command_buffer; //the buffer we will record into

	vk_util::DescriptorAllocator *dynamic_descriptor_allocator; //descriptor allocator for this frame

	//buffer that holds a single GPUCameraData to use when render
	AllocatedBufferUntyped camera_buffer;
	//	vk::DescriptorSet global_descriptor;
	//
	AllocatedBufferUntyped object_buffer;
	//	vk::DescriptorSet object_descriptor;

	AllocatedBufferUntyped indirect_buffer;

	vk_util::PushBuffer dynamic_data;
};

class RenderManager {
public:
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
	vk::RenderPass copy_pass;
	vk::RenderPass render_pass;
	vk::RenderPass shadow_pass;
	std::vector<vk::Framebuffer> framebuffers;

	// PIPELINE
	vk::PipelineLayout mesh_pipeline_layout;
	vk::Pipeline mesh_pipeline;

	vk::PipelineLayout blit_pipeline_layout;
	vk::Pipeline blit_pipeline;

	vk::Pipeline cull_pipeline;
	vk::PipelineLayout cull_layout;

	vk::Pipeline sparse_upload_pipeline;
	vk::PipelineLayout sparse_upload_layout;

	vk::Pipeline depth_reduce_pipeline;
	vk::PipelineLayout depth_reduce_layout;

	// MESHES
	Mesh triangle_mesh;
	Mesh box_mesh;

	// DEPTH
	AllocatedImage depth_image;

	vk::Sampler shadow_sampler;
	AllocatedImage shadow_image;

	vk::Format depth_format;

	// DESCRIPTORS
	vk_util::DescriptorAllocator *descriptor_allocator;
	vk_util::DescriptorLayoutCache *descriptor_layout_cache;

	vk::DescriptorSetLayout global_set_layout;
	vk::DescriptorSetLayout object_set_layout;
	vk::DescriptorSetLayout single_texture_set_layout;

	vk::PhysicalDeviceProperties gpu_properties;

	UploadContext upload_context;

	GPUSceneData scene_parameters;
	AllocatedBufferUntyped scene_parameter_buffer;

	vk::Extent2D window_extent;

	vk_util::MaterialSystem material_system;
	ShaderCache shader_cache;

	vk::Sampler depth_sampler;
	vk::ImageView depth_pyramid_mips[16] = {};
	AllocatedImage depth_pyramid;
	int depth_pyramid_width;
	int depth_pyramid_height;
	int depth_pyramid_levels;

	std::vector<vk::BufferMemoryBarrier> upload_barriers;
	std::vector<vk::BufferMemoryBarrier> cull_ready_barriers;
	std::vector<vk::BufferMemoryBarrier> post_cull_barriers;

	vk::Format render_format;
	AllocatedImage raw_render_image;
	vk::Sampler smooth_sampler;
	vk::Framebuffer forward_framebuffer;

	void init_vulkan(DisplayManager &display_manager);
	void init_swapchain(DisplayManager &display_manager);
	void init_commands();
	void init_forward_renderpass();
	void init_copy_renderpass();
	void init_framebuffers();
	void init_sync_structures();
	void init_descriptors();
	void init_pipelines();
	void init_scene();
	void init_imgui(GLFWwindow *window);

	void load_meshes();
	void upload_mesh(Mesh &mesh);

	bool load_compute_shader(const char *shader_path, vk::Pipeline &pipeline, vk::PipelineLayout &layout);

	[[nodiscard]] size_t pad_uniform_buffer_size(size_t original_size) const;

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
	RenderScene render_scene;
	std::unordered_map<std::string, Mesh> meshes;

	// texture stuff
	std::unordered_map<std::string, Texture> loaded_textures;

	Status startup(DisplayManager &display_manager, Camera *cam);
	void shutdown();

	//getter for the frame we are render to right now.
	FrameData &get_current_frame();

	Camera *camera = nullptr;

	// draw functions
	void ready_mesh_draw(vk::CommandBuffer cmd);
	void draw_objects_forward(vk::CommandBuffer cmd, RenderScene::MeshPass &pass);
	void execute_draw_commands(vk::CommandBuffer cmd, RenderScene::MeshPass &pass, vk::DescriptorSet object_data_set,
			std::vector<uint32_t> dynamic_offsets, vk::DescriptorSet global_set);
	void draw_objects_shadow(vk::CommandBuffer cmd, RenderScene::MeshPass &pass);
	void reduce_depth(vk::CommandBuffer cmd);
	void execute_compute_cull(vk::CommandBuffer cmd, RenderScene::MeshPass &pass, CullParams &params);
	void ready_cull_data(RenderScene::MeshPass &pass, vk::CommandBuffer cmd);

	void forward_pass(vk::CommandBuffer cmd);
	void copy_render_to_swapchain(vk::CommandBuffer cmd, uint32_t swapchain_image_index);

	[[nodiscard]] AllocatedBufferUntyped create_buffer(
			size_t alloc_size, vk::BufferUsageFlags usage, vma::MemoryUsage memory_usage) const;
	void reallocate_buffer(AllocatedBufferUntyped &buffer, size_t alloc_size, vk::BufferUsageFlags usage,
			vma::MemoryUsage memory_usage, vk::MemoryPropertyFlags required_flags = {});

	template <typename T> T *map_buffer(AllocatedBuffer<T> &buffer);
	void unmap_buffer(AllocatedBufferUntyped &buffer);

	void immediate_submit(std::function<void(vk::CommandBuffer cmd)> &&function);

	static std::string asset_path(std::string_view path);
	static std::string shader_path(std::string_view path);

	//returns nullptr if it can't be found
	Mesh *get_mesh(const std::string &name);

	void load_images();

	void draw(Camera &camera);
	void draw_objects(Camera &camera, vk::CommandBuffer cmd);
};

template <typename T> T *RenderManager::map_buffer(AllocatedBuffer<T> &buffer) {
	void *data;
	VK_CHECK(allocator.mapMemory(buffer.allocation, &data));
	return (T *)data;
}

#endif //SILENCE_RENDER_MANAGER_H
