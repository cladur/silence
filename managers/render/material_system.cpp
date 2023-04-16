#include "material_system.h"

#include "render/render_manager.h"
#include "vk_debug.h"
#include "vk_initializers.h"
#include "vk_shaders.h"
#include <vulkan/vulkan_enums.hpp>

vk::Pipeline PipelineBuilder::build_pipeline(vk::Device device, vk::RenderPass pass) {
	vertex_input_info = vk_init::vertex_input_state_create_info();
	//connect the pipeline builder vertex input info to the one we get from Vertex
	vertex_input_info.pVertexAttributeDescriptions = vertex_description.attributes.data();
	vertex_input_info.vertexAttributeDescriptionCount = (uint32_t)vertex_description.attributes.size();

	vertex_input_info.pVertexBindingDescriptions = vertex_description.bindings.data();
	vertex_input_info.vertexBindingDescriptionCount = (uint32_t)vertex_description.bindings.size();

	//make viewport state from our stored viewport and scissor.
	//at the moment we won't support multiple viewports or scissors
	vk::PipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
	viewport_state.pNext = nullptr;

	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	//setup dummy color blending. We aren't using transparent objects yet
	//the blending is just "no blend", but we do write to the color attachment
	vk::PipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
	color_blending.pNext = nullptr;

	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = vk::LogicOp::eCopy;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;

	//build the actual pipeline
	//we now use all the info structs we have been writing into this one to create the pipeline
	vk::GraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
	pipeline_info.pNext = nullptr;

	pipeline_info.stageCount = shader_stages.size();
	pipeline_info.pStages = shader_stages.data();
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.layout = pipeline_layout;
	pipeline_info.renderPass = pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.pDepthStencilState = &depth_stencil;

	vk::PipelineDynamicStateCreateInfo dynamic_state{};
	dynamic_state.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;

	std::vector<vk::DynamicState> dynamic_states;
	dynamic_states.push_back(vk::DynamicState::eViewport);
	dynamic_states.push_back(vk::DynamicState::eScissor);
	dynamic_states.push_back(vk::DynamicState::eDepthBias);
	dynamic_state.pDynamicStates = dynamic_states.data();
	dynamic_state.dynamicStateCount = (uint32_t)dynamic_states.size();

	pipeline_info.pDynamicState = &dynamic_state;

	//it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK
	//case
	vk::Pipeline new_pipeline;
	if (device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &new_pipeline) !=
			vk::Result::eSuccess) {
		SPDLOG_ERROR("failed to create pipeline");
		return VK_NULL_HANDLE; // failed to create graphics pipeline
	} else {
		return new_pipeline;
	}
}

vk::Pipeline ComputePipelineBuilder::build_pipeline(vk::Device device) const {
	vk::ComputePipelineCreateInfo pipeline_info{};
	pipeline_info.sType = vk::StructureType::eComputePipelineCreateInfo;
	pipeline_info.pNext = nullptr;

	pipeline_info.stage = shader_stage;
	pipeline_info.layout = pipeline_layout;

	vk::Pipeline new_pipeline;
	if (device.createComputePipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &new_pipeline) !=
			vk::Result::eSuccess) {
		SPDLOG_ERROR("Failed to build compute pipeline");
		return VK_NULL_HANDLE;
	} else {
		return new_pipeline;
	}
}

void PipelineBuilder::clear_vertex_input() {
	vertex_input_info.pVertexAttributeDescriptions = nullptr;
	vertex_input_info.vertexAttributeDescriptionCount = 0;

	vertex_input_info.pVertexBindingDescriptions = nullptr;
	vertex_input_info.vertexBindingDescriptionCount = 0;
}

void PipelineBuilder::set_shaders(struct ShaderEffect *effect) {
	shader_stages.clear();
	effect->fill_stages(shader_stages);

	pipeline_layout = effect->built_layout;
}

bool vk_util::MaterialData::operator==(const MaterialData &other) const {
	if (other.base_template != base_template || other.parameters != parameters ||
			other.textures.size() != textures.size()) {
		return false;
	} else {
		//binary compare textures
		bool comp = memcmp(other.textures.data(), textures.data(), textures.size() * sizeof(textures[0])) == 0;
		return comp;
	}
}

size_t vk_util::MaterialData::hash() const {
	using std::hash;
	using std::size_t;

	size_t result = hash<std::string>()(base_template);

	for (const auto &b : textures) {
		//pack the binding data into a single int64. Not fully correct but it's ok
		size_t texture_hash = (std::hash<size_t>()((size_t)(VkSampler)b.sampler) << 3) &&
				((std::hash<size_t>()((size_t)(VkImageView)b.image_view) >> 7));

		//shuffle the packed binding data and xor it with the main hash
		result ^= std::hash<size_t>()(texture_hash);
	}

	return result;
}

void vk_util::MaterialSystem::init(RenderManager *owner) {
	manager = owner;
	build_default_templates();
}

void vk_util::MaterialSystem::cleanup() {
}

ShaderEffect *build_effect(RenderManager *manager, std::string_view vertex_shader, std::string_view fragment_shader) {
	ShaderEffect::ReflectionOverrides overrides[] = { { "sceneData", vk::DescriptorType::eUniformBufferDynamic },
		{ "cameraData", vk::DescriptorType::eUniformBufferDynamic } };
	//textured defaultlit shader
	auto *effect = new ShaderEffect();

	effect->add_stage(manager->shader_cache.get_shader(RenderManager::shader_path(vertex_shader)),
			vk::ShaderStageFlagBits::eVertex);
	if (fragment_shader.size() > 2) {
		effect->add_stage(manager->shader_cache.get_shader(RenderManager::shader_path(fragment_shader)),
				vk::ShaderStageFlagBits::eFragment);
	}

	effect->reflect_layout(manager->device, overrides, 2);

	return effect;
}

void vk_util::MaterialSystem::build_default_templates() {
	fill_builders();

	//default effects
	ShaderEffect *textured_lit = build_effect(manager, "tri_mesh.vert.spv", "textured_lit.frag.spv");
	// ShaderEffect *default_lit = build_effect(manager, "tri_mesh_ssbo_instanced.vert.spv", "default_lit.frag.spv");
	// ShaderEffect *opaque_shadowcast = build_effect(manager, "tri_mesh_ssbo_instanced_shadowcast.vert.spv", "");

	//passes
	ShaderPass *textured_lit_pass = build_shader(manager->render_pass, forward_builder, textured_lit);
	// ShaderPass *default_lit_pass = build_shader(manager->render_pass, forward_builder, default_lit);
	// ShaderPass *opaque_shadowcast_pass = build_shader(manager->shadow_pass, shadow_builder, opaque_shadowcast);

	{
		EffectTemplate default_textured{};
		default_textured.pass_shaders[MeshpassType::Transparency] = nullptr;
		// default_textured.pass_shaders[MeshpassType::DirectionalShadow] = opaque_shadowcast_pass;
		default_textured.pass_shaders[MeshpassType::Forward] = textured_lit_pass;

		default_textured.default_parameters = nullptr;
		default_textured.transparency = assets::TransparencyMode::Opaque;

		template_cache["texturedPBR_opaque"] = default_textured;
	}
	/*
	{
		PipelineBuilder transparent_forward = forward_builder;

		transparent_forward.color_blend_attachment.blendEnable = VK_TRUE;
		transparent_forward.color_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
		transparent_forward.color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
		transparent_forward.color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eOne;

		//transparent_forward.color_blend_attachment.colorBlendOp = VK_BLEND_OP_OVERLAY_EXT;
		transparent_forward.color_blend_attachment.colorWriteMask =
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;

		transparent_forward.depth_stencil.depthWriteEnable = false;

		transparent_forward.rasterizer.cullMode = vk::CullModeFlagBits::eNone;
		//passes
		ShaderPass *transparent_lit_pass = build_shader(manager->render_pass, transparent_forward, textured_lit);

		EffectTemplate default_textured{};
		default_textured.pass_shaders[MeshpassType::Transparency] = transparent_lit_pass;
		default_textured.pass_shaders[MeshpassType::DirectionalShadow] = nullptr;
		default_textured.pass_shaders[MeshpassType::Forward] = nullptr;

		default_textured.default_parameters = nullptr;
		default_textured.transparency = assets::TransparencyMode::Transparent;

		template_cache["texturedPBR_transparent"] = default_textured;
	}

	{
		EffectTemplate default_colored{};

		default_colored.pass_shaders[MeshpassType::Transparency] = nullptr;
		// default_colored.pass_shaders[MeshpassType::DirectionalShadow] = opaque_shadowcast_pass;
		// default_colored.pass_shaders[MeshpassType::Forward] = default_lit_pass;
		default_colored.default_parameters = nullptr;
		default_colored.transparency = assets::TransparencyMode::Opaque;
		template_cache["colored_opaque"] = default_colored;
	}
*/
}

vk_util::ShaderPass *vk_util::MaterialSystem::build_shader(
		vk::RenderPass render_pass, PipelineBuilder &builder, ShaderEffect *effect) {
	auto *pass = new vk_util::ShaderPass();

	pass->effect = effect;
	pass->layout = effect->built_layout;

	PipelineBuilder pipbuilder = builder;

	pipbuilder.set_shaders(effect);

	pass->pipeline = pipbuilder.build_pipeline(manager->device, render_pass);
	VkDebug::set_name(pass->pipeline, "Material pass pipeline");
	manager->main_deletion_queue.push_function([=, this]() { manager->device.destroyPipeline(pass->pipeline); });

	return pass;
}

vk_util::Material *vk_util::MaterialSystem::build_material(
		const std::string &material_name, const vk_util::MaterialData &info) {
	vk_util::Material *mat;
	//search material in the cache first in case its already built
	auto it = material_cache.find(info);
	if (it != material_cache.end()) {
		mat = (*it).second;
		materials[material_name] = mat;
	} else {
		//need to build the material
		auto *new_mat = new Material();
		new_mat->original = &template_cache[info.base_template];
		new_mat->parameters = info.parameters;
		//not handled yet
		new_mat->pass_sets[MeshpassType::DirectionalShadow] = VK_NULL_HANDLE;
		new_mat->textures = info.textures;

		auto db = vk_util::DescriptorBuilder::begin(manager->descriptor_layout_cache, manager->descriptor_allocator);

		vk::DescriptorImageInfo image_buffer_infos[info.textures.size()];

		for (int i = 0; i < info.textures.size(); i++) {
			vk::DescriptorImageInfo &image_buffer_info = image_buffer_infos[i];
			image_buffer_info.sampler = info.textures[i].sampler;
			image_buffer_info.imageView = info.textures[i].image_view;
			image_buffer_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			db.bind_image(i, &image_buffer_info, vk::DescriptorType::eCombinedImageSampler,
					vk::ShaderStageFlagBits::eFragment);
		}

		db.build(new_mat->pass_sets[MeshpassType::Forward]);
		SPDLOG_INFO("Built New Material {}", material_name);
		//add material to cache
		material_cache[info] = (new_mat);
		mat = new_mat;
		materials[material_name] = mat;
	}

	return mat;
}

vk_util::Material *vk_util::MaterialSystem::get_material(const std::string &material_name) {
	auto it = materials.find(material_name);
	if (it != materials.end()) {
		return (*it).second;
	} else {
		return nullptr;
	}
}

void vk_util::MaterialSystem::fill_builders() {
	{
		shadow_builder.vertex_description = Vertex::get_vertex_description();

		shadow_builder.input_assembly = vk_init::input_assembly_create_info(vk::PrimitiveTopology::eTriangleList);

		shadow_builder.rasterizer = vk_init::rasterization_state_create_info(vk::PolygonMode::eFill);
		shadow_builder.rasterizer.cullMode = vk::CullModeFlagBits::eFront;
		shadow_builder.rasterizer.depthBiasEnable = VK_TRUE;

		shadow_builder.multisampling = vk_init::multisampling_state_create_info();
		shadow_builder.color_blend_attachment = vk_init::color_blend_attachment_state();

		//default depthtesting
		shadow_builder.depth_stencil = vk_init::depth_stencil_create_info(true, true, vk::CompareOp::eLess);
	}
	{
		forward_builder.vertex_description = Vertex::get_vertex_description();

		forward_builder.input_assembly = vk_init::input_assembly_create_info(vk::PrimitiveTopology::eTriangleList);

		forward_builder.rasterizer = vk_init::rasterization_state_create_info(vk::PolygonMode::eFill);
		forward_builder.rasterizer.cullMode = vk::CullModeFlagBits::eNone; //BACK_BIT;

		forward_builder.multisampling = vk_init::multisampling_state_create_info();

		forward_builder.color_blend_attachment = vk_init::color_blend_attachment_state();

		//default depthtesting
		forward_builder.depth_stencil = vk_init::depth_stencil_create_info(true, true, vk::CompareOp::eGreaterOrEqual);

		// OUR STUFF
		forward_builder.viewport.x = 0.0f;
		forward_builder.viewport.y = (float)manager->window_extent.height;
		forward_builder.viewport.width = (float)manager->window_extent.width;
		forward_builder.viewport.height = -(float)manager->window_extent.height;
		forward_builder.viewport.minDepth = 0.0f;
		forward_builder.viewport.maxDepth = 1.0f;

		forward_builder.scissor.offset = vk::Offset2D(0, 0);
		forward_builder.scissor.extent = manager->window_extent;
	}
}
