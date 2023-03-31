#include "pipeline_builder.h"

vk::Pipeline PipelineBuilder::build_pipeline(vk::Device device, vk::RenderPass pass) {
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
