#ifndef SILENCE_PIPELINE_BUILDER_H
#define SILENCE_PIPELINE_BUILDER_H

#include <vector>

#include <vulkan/vulkan.hpp>

class PipelineBuilder {
public:
	std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
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
};

#endif //SILENCE_PIPELINE_BUILDER_H
