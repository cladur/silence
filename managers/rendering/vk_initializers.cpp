#include "vk_initializers.h"
vk::CommandPoolCreateInfo vk_init::command_pool_create_info(
		uint32_t queue_family_index, vk::CommandPoolCreateFlags flags) {
	vk::CommandPoolCreateInfo info = {};
	info.sType = vk::StructureType::eCommandPoolCreateInfo;
	info.pNext = nullptr;

	info.queueFamilyIndex = queue_family_index;
	info.flags = flags;
	return info;
}

vk::CommandBufferAllocateInfo vk_init::command_buffer_allocate_info(
		vk::CommandPool pool, uint32_t count, vk::CommandBufferLevel level) {
	vk::CommandBufferAllocateInfo info = {};
	info.sType = vk::StructureType::eCommandBufferAllocateInfo;
	info.pNext = nullptr;

	info.commandPool = pool;
	info.commandBufferCount = count;
	info.level = level;
	return info;
}
vk::PipelineShaderStageCreateInfo vk_init::pipeline_shader_stage_create_info(
		vk::ShaderStageFlagBits stage, vk::ShaderModule shader_module) {
	vk::PipelineShaderStageCreateInfo info = {};
	info.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
	info.pNext = nullptr;

	//shader stage
	info.stage = stage;
	//module containing the code for this shader stage
	info.module = shader_module;
	//the entry point of the shader
	info.pName = "main";
	return info;
}

vk::PipelineVertexInputStateCreateInfo vk_init::vertex_input_state_create_info() {
	vk::PipelineVertexInputStateCreateInfo info = {};
	info.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
	info.pNext = nullptr;

	//no vertex bindings or attributes
	info.vertexBindingDescriptionCount = 0;
	info.vertexAttributeDescriptionCount = 0;
	return info;
}

vk::PipelineInputAssemblyStateCreateInfo vk_init::input_assembly_create_info(vk::PrimitiveTopology topology) {
	vk::PipelineInputAssemblyStateCreateInfo info = {};
	info.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
	info.pNext = nullptr;

	info.topology = topology;
	//we are not going to use primitive restart on the entire tutorial so leave it on false
	info.primitiveRestartEnable = VK_FALSE;
	return info;
}

vk::PipelineRasterizationStateCreateInfo vk_init::rasterization_state_create_info(vk::PolygonMode polygon_mode) {
	vk::PipelineRasterizationStateCreateInfo info = {};
	info.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
	info.pNext = nullptr;

	info.depthClampEnable = VK_FALSE;
	//discards all primitives before the rasterization stage if enabled which we don't want
	info.rasterizerDiscardEnable = VK_FALSE;

	info.polygonMode = polygon_mode;
	info.lineWidth = 1.0f;
	//no backface cull
	info.cullMode = vk::CullModeFlagBits::eNone;
	;
	info.frontFace = vk::FrontFace::eClockwise;
	//no depth bias
	info.depthBiasEnable = VK_FALSE;
	info.depthBiasConstantFactor = 0.0f;
	info.depthBiasClamp = 0.0f;
	info.depthBiasSlopeFactor = 0.0f;

	return info;
}

vk::PipelineMultisampleStateCreateInfo vk_init::multisampling_state_create_info() {
	vk::PipelineMultisampleStateCreateInfo info = {};
	info.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
	info.pNext = nullptr;

	info.sampleShadingEnable = VK_FALSE;
	//multisampling defaulted to no multisampling (1 sample per pixel)
	info.rasterizationSamples = vk::SampleCountFlagBits::e1;
	info.minSampleShading = 1.0f;
	info.pSampleMask = nullptr;
	info.alphaToCoverageEnable = VK_FALSE;
	info.alphaToOneEnable = VK_FALSE;
	return info;
}

vk::PipelineColorBlendAttachmentState vk_init::color_blend_attachment_state() {
	vk::PipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	color_blend_attachment.blendEnable = VK_FALSE;
	return color_blend_attachment;
}
vk::PipelineLayoutCreateInfo vk_init::pipeline_layout_create_info() {
	vk::PipelineLayoutCreateInfo info = {};
	info.sType = vk::StructureType::ePipelineLayoutCreateInfo;
	info.pNext = nullptr;

	//empty defaults
	info.setLayoutCount = 0;
	info.pSetLayouts = nullptr;
	info.pushConstantRangeCount = 0;
	info.pPushConstantRanges = nullptr;
	return info;
}

vk::FenceCreateInfo vk_init::fence_create_info(vk::FenceCreateFlags flags) {
	vk::FenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = vk::StructureType::eFenceCreateInfo;
	fenceCreateInfo.pNext = nullptr;
	fenceCreateInfo.flags = flags;
	return fenceCreateInfo;
}

vk::SemaphoreCreateInfo vk_init::semaphore_create_info(vk::SemaphoreCreateFlags flags) {
	vk::SemaphoreCreateInfo semCreateInfo = {};
	semCreateInfo.sType = vk::StructureType::eSemaphoreCreateInfo;
	semCreateInfo.pNext = nullptr;
	semCreateInfo.flags = flags;
	return semCreateInfo;
}
