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
	vk::FenceCreateInfo fence_create_info = {};
	fence_create_info.sType = vk::StructureType::eFenceCreateInfo;
	fence_create_info.pNext = nullptr;
	fence_create_info.flags = flags;
	return fence_create_info;
}

vk::SemaphoreCreateInfo vk_init::semaphore_create_info(vk::SemaphoreCreateFlags flags) {
	vk::SemaphoreCreateInfo sem_create_info = {};
	sem_create_info.sType = vk::StructureType::eSemaphoreCreateInfo;
	sem_create_info.pNext = nullptr;
	sem_create_info.flags = flags;
	return sem_create_info;
}

vk::ImageCreateInfo vk_init::image_create_info(
		vk::Format format, vk::ImageUsageFlags usage_flags, vk::Extent3D extent) {
	vk::ImageCreateInfo info = {};
	info.sType = vk::StructureType::eImageCreateInfo;
	info.pNext = nullptr;

	info.imageType = vk::ImageType::e2D;

	info.format = format;
	info.extent = extent;

	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = vk::SampleCountFlagBits::e1;
	info.tiling = vk::ImageTiling::eOptimal;
	info.usage = usage_flags;

	return info;
}

vk::ImageViewCreateInfo vk_init::image_view_create_info(
		vk::Format format, vk::Image image, vk::ImageAspectFlags aspect_flags) {
	//build an image-view for the depth image to use for rendering
	vk::ImageViewCreateInfo info = {};
	info.sType = vk::StructureType::eImageViewCreateInfo;
	info.pNext = nullptr;

	info.viewType = vk::ImageViewType::e2D;
	info.image = image;
	info.format = format;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = 1;
	info.subresourceRange.aspectMask = aspect_flags;

	return info;
}
vk::PipelineDepthStencilStateCreateInfo vk_init::depth_stencil_create_info(
		bool depth_test, bool depth_write, vk::CompareOp compare_op) {
	vk::PipelineDepthStencilStateCreateInfo info = {};
	info.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
	info.pNext = nullptr;

	info.depthTestEnable = depth_test ? VK_TRUE : VK_FALSE;
	info.depthWriteEnable = depth_write ? VK_TRUE : VK_FALSE;
	info.depthCompareOp = depth_test ? compare_op : vk::CompareOp::eAlways;
	info.depthBoundsTestEnable = VK_FALSE;
	info.minDepthBounds = 0.0f; // Optional
	info.maxDepthBounds = 1.0f; // Optional
	info.stencilTestEnable = VK_FALSE;

	return info;
}

vk::DescriptorSetLayoutBinding vk_init::descriptor_set_layout_binding(
		vk::DescriptorType type, vk::ShaderStageFlags stage_flags, uint32_t binding) {
	vk::DescriptorSetLayoutBinding setbind = {};
	setbind.binding = binding;
	setbind.descriptorCount = 1;
	setbind.descriptorType = type;
	setbind.pImmutableSamplers = nullptr;
	setbind.stageFlags = stage_flags;

	return setbind;
}

vk::WriteDescriptorSet vk_init::write_descriptor_buffer(
		vk::DescriptorType type, vk::DescriptorSet dst_set, vk::DescriptorBufferInfo *buffer_info, uint32_t binding) {
	vk::WriteDescriptorSet write = {};
	write.sType = vk::StructureType::eWriteDescriptorSet;
	write.pNext = nullptr;

	write.dstBinding = binding;
	write.dstSet = dst_set;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pBufferInfo = buffer_info;

	return write;
}

vk::CommandBufferBeginInfo vk_init::command_buffer_begin_info(vk::CommandBufferUsageFlags flags) {
	vk::CommandBufferBeginInfo info = {};
	info.sType = vk::StructureType::eCommandBufferBeginInfo;
	info.pNext = nullptr;

	info.pInheritanceInfo = nullptr;
	info.flags = flags;
	return info;
}

vk::SubmitInfo vk_init::submit_info(vk::CommandBuffer *cmd) {
	vk::SubmitInfo info = {};
	info.sType = vk::StructureType::eSubmitInfo;
	info.pNext = nullptr;

	info.waitSemaphoreCount = 0;
	info.pWaitSemaphores = nullptr;
	info.pWaitDstStageMask = nullptr;
	info.commandBufferCount = 1;
	info.pCommandBuffers = cmd;
	info.signalSemaphoreCount = 0;
	info.pSignalSemaphores = nullptr;

	return info;
}

vk::SamplerCreateInfo vk_init::sampler_create_info(vk::Filter filters, vk::SamplerAddressMode sampler_address_mode) {
	vk::SamplerCreateInfo info = {};
	info.sType = vk::StructureType::eSamplerCreateInfo;
	info.pNext = nullptr;

	info.magFilter = filters;
	info.minFilter = filters;
	info.addressModeU = sampler_address_mode;
	info.addressModeV = sampler_address_mode;
	info.addressModeW = sampler_address_mode;

	return info;
}

vk::WriteDescriptorSet vk_init::write_descriptor_image(
		vk::DescriptorType type, vk::DescriptorSet dst_set, vk::DescriptorImageInfo *image_info, uint32_t binding) {
	vk::WriteDescriptorSet write = {};
	write.sType = vk::StructureType::eWriteDescriptorSet;
	write.pNext = nullptr;

	write.dstBinding = binding;
	write.dstSet = dst_set;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pImageInfo = image_info;

	return write;
}
