#ifndef SILENCE_VK_INITIALIZERS_H
#define SILENCE_VK_INITIALIZERS_H

#include "vulkan/vulkan.hpp"

namespace vk_init {

vk::CommandPoolCreateInfo command_pool_create_info(uint32_t queue_family_index, vk::CommandPoolCreateFlags flags = {});

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

vk::ImageCreateInfo image_create_info(vk::Format format, vk::ImageUsageFlags usage_flags, vk::Extent3D extent);

vk::ImageViewCreateInfo image_view_create_info(vk::Format format, vk::Image image, vk::ImageAspectFlags aspect_flags);

vk::PipelineDepthStencilStateCreateInfo depth_stencil_create_info(
		bool depth_test, bool depth_write, vk::CompareOp compare_op);

vk::DescriptorSetLayoutBinding descriptor_set_layout_binding(
		vk::DescriptorType type, vk::ShaderStageFlags stage_flags, uint32_t binding);

vk::WriteDescriptorSet write_descriptor_buffer(
		vk::DescriptorType type, vk::DescriptorSet dst_set, vk::DescriptorBufferInfo *buffer_info, uint32_t binding);

vk::CommandBufferBeginInfo command_buffer_begin_info(vk::CommandBufferUsageFlags flags = {});

vk::SubmitInfo submit_info(vk::CommandBuffer *cmd);

vk::SamplerCreateInfo sampler_create_info(
		vk::Filter filters, vk::SamplerAddressMode sampler_address_mode = vk::SamplerAddressMode::eRepeat);

vk::WriteDescriptorSet write_descriptor_image(
		vk::DescriptorType type, vk::DescriptorSet dst_set, vk::DescriptorImageInfo *image_info, uint32_t binding);

vk::BufferMemoryBarrier buffer_barrier(vk::Buffer buffer, uint32_t queue);

vk::ImageMemoryBarrier image_barrier(vk::Image image, vk::AccessFlags src_access_mask, vk::AccessFlags dst_access_mask,
		vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::ImageAspectFlags aspect_mask);

vk::RenderPassBeginInfo renderpass_begin_info(
		vk::RenderPass render_pass, vk::Extent2D window_extent, vk::Framebuffer framebuffer);

vk::FramebufferCreateInfo framebuffer_create_info(vk::RenderPass render_pass, vk::Extent2D extent);

vk::PresentInfoKHR present_info();

} //namespace vk_init

#endif //SILENCE_VK_INITIALIZERS_H
