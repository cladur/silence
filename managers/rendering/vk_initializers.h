#ifndef SILENCE_VK_INITIALIZERS_H
#define SILENCE_VK_INITIALIZERS_H

#include "vulkan/vulkan.hpp"

namespace vk_init {

vk::CommandPoolCreateInfo command_pool_create_info(uint32_t queue_family_index, vk::CommandPoolCreateFlags flags);

vk::CommandBufferAllocateInfo command_buffer_allocate_info(
		vk::CommandPool pool, uint32_t count = 1, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

vk::PipelineShaderStageCreateInfo pipeline_shader_stage_create_info(
		vk::ShaderStageFlagBits stage, vk::ShaderModule shader_module);

vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info();

vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info(vk::PrimitiveTopology topology);

vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info(vk::PolygonMode polygon_mode);

vk::PipelineMultisampleStateCreateInfo multisampling_state_create_info();

vk::PipelineColorBlendAttachmentState color_blend_attachment_state();

vk::PipelineLayoutCreateInfo pipeline_layout_create_info();

vk::FenceCreateInfo fence_create_info(vk::FenceCreateFlags flags = {});
vk::SemaphoreCreateInfo semaphore_create_info(vk::SemaphoreCreateFlags flags = {});

} //namespace vk_init

#endif //SILENCE_VK_INITIALIZERS_H
