#ifndef SILENCE_VK_SHADERS_H
#define SILENCE_VK_SHADERS_H

#include <vulkan/vulkan.hpp>

#include "vk_descriptors.h"
#include "vk_types.h"

struct ShaderModule {
	std::vector<uint32_t> code;
	vk::ShaderModule module;
};
namespace vk_util {

//loads a shader module from a spir-v file. Returns false if it errors
bool load_shader_module(vk::Device device, const char *file_path, ShaderModule *out_shader_module);

uint32_t hash_descriptor_layout_info(vk::DescriptorSetLayoutCreateInfo *info);
} //namespace vk_util

class RenderManager;

// Holds all shader related state that pipeline needs to be built.
struct ShaderEffect {
	struct ReflectionOverrides {
		const char *name;
		vk::DescriptorType overriden_type;
	};

	void add_stage(ShaderModule *shader_module, vk::ShaderStageFlagBits stage);

	void reflect_layout(vk::Device device, ReflectionOverrides *overrides, int override_count);

	void fill_stages(std::vector<vk::PipelineShaderStageCreateInfo> &pipeline_stages);
	vk::PipelineLayout built_layout;

	struct ReflectedBinding {
		uint32_t set;
		uint32_t binding;
		vk::DescriptorType type;
	};
	std::unordered_map<std::string, ReflectedBinding> bindings;
	std::array<vk::DescriptorSetLayout, 4> set_layouts;
	std::array<uint32_t, 4> set_hashes;

private:
	struct ShaderStage {
		ShaderModule *shader_module;
		vk::ShaderStageFlagBits stage;
	};

	std::vector<ShaderStage> stages;
};

struct ShaderDescriptorBinder {
	struct BufferWriteDescriptor {
		int dst_set;
		int dst_binding;
		vk::DescriptorType descriptor_type;
		vk::DescriptorBufferInfo buffer_info;

		uint32_t dynamic_offset;
	};

	void bind_buffer(const char *name, const vk::DescriptorBufferInfo &buffer_info);

	void bind_dynamic_buffer(const char *name, uint32_t offset, const vk::DescriptorBufferInfo &buffer_info);

	void apply_binds(vk::CommandBuffer cmd);

	//void build_sets(vk::Device device, vk::DescriptorPool allocator);
	void build_sets(vk::Device device, vk_util::DescriptorAllocator &allocator);

	void set_shader(ShaderEffect *new_shader);

	std::array<vk::DescriptorSet, 4> cached_descriptor_sets;

private:
	struct DynOffsets {
		std::array<uint32_t, 16> offsets;
		uint32_t count{ 0 };
	};
	std::array<DynOffsets, 4> set_offsets;

	ShaderEffect *shaders{ nullptr };
	std::vector<BufferWriteDescriptor> buffer_writes;
};

class ShaderCache {
public:
	ShaderModule *get_shader(const std::string &path);

	void init(vk::Device new_device) {
		device = new_device;
	};

private:
	vk::Device device;
	std::unordered_map<std::string, ShaderModule> module_cache;
};

#endif //SILENCE_VK_SHADERS_H