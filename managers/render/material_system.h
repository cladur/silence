#ifndef SILENCE_MATERIAL_SYSTEM_H
#define SILENCE_MATERIAL_SYSTEM_H

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

#include "assets/material_asset.h"
#include "vk_mesh.h"

struct ShaderEffect;
class RenderManager;
struct ShaderParameters {};

class PipelineBuilder {
public:
	std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
	VertexInputDescription vertex_description;
	vk::PipelineVertexInputStateCreateInfo vertex_input_info;
	vk::PipelineInputAssemblyStateCreateInfo input_assembly;
	vk::Viewport viewport;
	vk::Rect2D scissor;
	vk::PipelineRasterizationStateCreateInfo rasterizer;
	vk::PipelineColorBlendAttachmentState color_blend_attachment;
	vk::PipelineMultisampleStateCreateInfo multisampling;
	vk::PipelineLayout pipeline_layout;
	vk::PipelineDepthStencilStateCreateInfo depth_stencil;

	vk::Pipeline build_pipeline(vk::Device device, vk::RenderPass pass);
	void clear_vertex_input();

	void set_shaders(ShaderEffect *effect);
};

enum class VertexAttributeTemplate { DefaultVertex, DefaultVertexPosOnly };

class EffectBuilder {
	VertexAttributeTemplate vertex_attrib = VertexAttributeTemplate::DefaultVertex;
	ShaderEffect *effect{ nullptr };

	vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
	vk::PipelineRasterizationStateCreateInfo rasterizer_info{};
	vk::PipelineColorBlendAttachmentState color_blend_attachment_info{};
	vk::PipelineDepthStencilStateCreateInfo depth_stencil_info{};
};

class ComputePipelineBuilder {
public:
	vk::PipelineShaderStageCreateInfo shader_stage;
	vk::PipelineLayout pipeline_layout;
	[[nodiscard]] vk::Pipeline build_pipeline(vk::Device device) const;
};

namespace vk_util {

// Essentially, the built version of ShaderEffect.
struct ShaderPass {
	ShaderEffect *effect{ nullptr };
	vk::Pipeline pipeline{ VK_NULL_HANDLE };
	vk::PipelineLayout layout{ VK_NULL_HANDLE };
};

struct SampledTexture {
	vk::Sampler sampler;
	vk::ImageView image_view;
};

template <typename T> struct PerPassData {
public:
	T &operator[](MeshpassType pass) {
		switch (pass) {
			case MeshpassType::Forward:
				return data[0];
			case MeshpassType::Transparency:
				return data[1];
			case MeshpassType::DirectionalShadow:
				return data[2];
			default:
				break;
		}
		assert(false);
		return data[0];
	};

	void clear(T &&val) {
		for (int i = 0; i < 3; i++) {
			data[i] = val;
		}
	}

private:
	std::array<T, 3> data;
};

struct EffectTemplate {
	PerPassData<ShaderPass *> pass_shaders;

	ShaderParameters *default_parameters;
	assets::TransparencyMode transparency;
};

struct MaterialData {
	std::vector<SampledTexture> textures;
	ShaderParameters *parameters;
	std::string base_template;

	bool operator==(const MaterialData &other) const;

	[[nodiscard]] size_t hash() const;
};

struct Material {
	EffectTemplate *original;
	PerPassData<vk::DescriptorSet> pass_sets;

	std::vector<SampledTexture> textures;

	ShaderParameters *parameters;

	Material &operator=(const Material &other) = default;
};

class MaterialSystem {
public:
	void init(RenderManager *owner);
	void cleanup();

	void build_default_templates();

	ShaderPass *build_shader(vk::RenderPass render_pass, PipelineBuilder &builder, ShaderEffect *effect);

	Material *build_material(const std::string &material_name, const MaterialData &info);
	Material *get_material(const std::string &material_name);

	void fill_builders();

private:
	struct MaterialInfoHash {
		std::size_t operator()(const MaterialData &k) const {
			return k.hash();
		}
	};

	PipelineBuilder forward_builder;
	PipelineBuilder shadow_builder;

	std::unordered_map<std::string, EffectTemplate> template_cache;
	std::unordered_map<std::string, Material *> materials;
	std::unordered_map<MaterialData, Material *, MaterialInfoHash> material_cache;
	RenderManager *manager;
};

} //namespace vk_util

#endif //SILENCE_MATERIAL_SYSTEM_H
