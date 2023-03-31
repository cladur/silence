#include "render_manager.h"
#include "display_manager.h"

#include <fstream>

#include <glm/gtx/transform.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "vulkan-memory-allocator-hpp/vk_mem_alloc.hpp"

#include "rendering/pipeline_builder.h"
#include "rendering/vk_initializers.h"
#include "rendering/vk_textures.h"
#include <VkBootstrap.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_enums.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

void RenderManager::init_vulkan(DisplayManager &display_manager) {
	vkb::InstanceBuilder builder;
	auto inst_ret = builder.set_app_name("Silence Vulkan Application")
							.request_validation_layers()
							.use_default_debug_messenger()
							.require_api_version(1, 1, 0)
							.build();

	if (!inst_ret) {
		SPDLOG_ERROR("Failed to create Vulkan Instance. Error: {}", inst_ret.error().message());
	}

	auto vkb_inst = inst_ret.value();

	//store the instance
	instance = vkb_inst.instance;
	//store the debug messenger
	debug_messenger = vkb_inst.debug_messenger;

	surface = display_manager.create_surface(vkb_inst.instance);

	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	auto physical_device_ret = selector.set_minimum_version(1, 1).set_surface(surface).select();

	if (!physical_device_ret) {
		SPDLOG_ERROR("Failed to find a suitable GPU. Error: {}", physical_device_ret.error().message());
	}

	vkb::PhysicalDevice physical_device = physical_device_ret.value();

	//create the final Vulkan device
	vkb::DeviceBuilder device_builder{ physical_device };

	vk::PhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {};
	shader_draw_parameters_features.sType = vk::StructureType::ePhysicalDeviceShaderDrawParametersFeatures;
	shader_draw_parameters_features.pNext = nullptr;
	shader_draw_parameters_features.shaderDrawParameters = VK_TRUE;
	vkb::Device vkb_device = device_builder.add_pNext(&shader_draw_parameters_features).build().value();

	// Get the VkDevice handle used in the rest of a Vulkan application
	device = vkb_device.device;
	chosen_gpu = physical_device.physical_device;

	// use vkbootstrap to get a Graphics queue
	graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
	graphics_queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

	//initialize the memory allocator
	vma::AllocatorCreateInfo allocator_info = {};
	allocator_info.physicalDevice = chosen_gpu;
	allocator_info.device = device;
	allocator_info.instance = instance;
	VK_CHECK(vma::createAllocator(&allocator_info, &allocator));

	gpu_properties = vkb_device.physical_device.properties;

	main_deletion_queue.push_function([=, this]() { allocator.destroy(); });
}

void RenderManager::init_swapchain(DisplayManager &display_manager) {
	auto window_size = display_manager.get_framebuffer_size();
	window_extent.width = window_size.first;
	window_extent.height = window_size.second;

	vkb::SwapchainBuilder swapchain_builder{ chosen_gpu, device, surface };

	auto vkb_swapchain = swapchain_builder
								 .use_default_format_selection()
								 // We use VSync present mode for now
								 // TODO: make this configurable if we want to uncap FPS in future
								 .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
								 .set_desired_extent(window_extent.width, window_extent.height)
								 .build()
								 .value();

	//store swapchain and its related images
	swapchain = vkb_swapchain.swapchain;
	auto temp_swapchain_images = vkb_swapchain.get_images().value();
	swapchain_images = std::vector<vk::Image>(temp_swapchain_images.begin(), temp_swapchain_images.end());
	auto temp_swapchain_image_views = vkb_swapchain.get_image_views().value();
	swapchain_image_views =
			std::vector<vk::ImageView>(temp_swapchain_image_views.begin(), temp_swapchain_image_views.end());

	swapchain_image_format = static_cast<vk::Format>(vkb_swapchain.image_format);

	main_deletion_queue.push_function([=, this]() { device.destroySwapchainKHR(swapchain); });

	//depth image size will match the window
	vk::Extent3D depth_image_extent = { window_extent.width, window_extent.height, 1 };

	//hardcoding the depth format to 32-bit float
	depth_format = vk::Format::eD32Sfloat;

	//the depth image will be an image with the format we selected and Depth Attachment usage flag
	vk::ImageCreateInfo dimg_info = vk_init::image_create_info(
			depth_format, vk::ImageUsageFlagBits::eDepthStencilAttachment, depth_image_extent);

	//for the depth image, we want to allocate it from GPU local memory
	vma::AllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = vma::MemoryUsage::eGpuOnly;
	dimg_allocinfo.requiredFlags = vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eDeviceLocal);

	//allocate and create the image
	VK_CHECK(allocator.createImage(&dimg_info, &dimg_allocinfo, &depth_image.image, &depth_image.allocation, nullptr));

	//build an image-view for the depth image to use for rendering
	vk::ImageViewCreateInfo dview_info =
			vk_init::image_view_create_info(depth_format, depth_image.image, vk::ImageAspectFlagBits::eDepth);

	VK_CHECK(device.createImageView(&dview_info, nullptr, &depth_image_view));

	//add to deletion queues
	main_deletion_queue.push_function([=, this]() {
		device.destroyImageView(depth_image_view);
		allocator.destroyImage(depth_image.image, depth_image.allocation);
	});
}

void RenderManager::init_commands() {
	//create a command pool for commands submitted to the graphics queue.
	vk::CommandPoolCreateInfo command_pool_info = vk_init::command_pool_create_info(
			graphics_queue_family, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	for (auto &frame : frames) {
		auto command_pool_res = device.createCommandPool(command_pool_info);

		VK_CHECK(command_pool_res.result);

		frame.command_pool = command_pool_res.value;

		//allocate the default command buffer that we will use for rendering
		vk::CommandBufferAllocateInfo cmd_alloc_info = vk_init::command_buffer_allocate_info(frame.command_pool, 1);

		VK_CHECK(device.allocateCommandBuffers(&cmd_alloc_info, &frame.main_command_buffer));

		main_deletion_queue.push_function([=, this]() { device.destroyCommandPool(frame.command_pool); });
	}

	vk::CommandPoolCreateInfo upload_command_pool_info = vk_init::command_pool_create_info(graphics_queue_family);
	//create pool for upload context
	VK_CHECK(device.createCommandPool(&upload_command_pool_info, nullptr, &upload_context.command_pool));

	main_deletion_queue.push_function([=, this]() { device.destroyCommandPool(upload_context.command_pool); });

	//allocate the default command buffer that we will use for the instant commands
	vk::CommandBufferAllocateInfo cmd_alloc_info =
			vk_init::command_buffer_allocate_info(upload_context.command_pool, 1);

	VK_CHECK(device.allocateCommandBuffers(&cmd_alloc_info, &upload_context.command_buffer));
}

void RenderManager::init_default_renderpass() {
	// the renderpass will use this color attachment.
	vk::AttachmentDescription color_attachment = {};
	//the attachment will have the format needed by the swapchain
	color_attachment.format = swapchain_image_format;
	//1 sample, we won't be doing MSAA
	color_attachment.samples = vk::SampleCountFlagBits::e1;
	// we Clear when this attachment is loaded
	color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
	// we keep the attachment stored when the renderpass ends
	color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
	//we don't care about stencil
	color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

	//we don't know or care about the starting layout of the attachment
	color_attachment.initialLayout = vk::ImageLayout::eUndefined;

	//after the renderpass ends, the image has to be on a layout ready for display
	color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference color_attachment_ref = {};
	//attachment number will index into the pAttachments array in the parent renderpass itself
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentDescription depth_attachment = {};
	// Depth attachment
	depth_attachment.format = depth_format;
	depth_attachment.samples = vk::SampleCountFlagBits::e1;
	depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
	depth_attachment.storeOp = vk::AttachmentStoreOp::eStore;
	depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
	depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
	depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	//we are going to create 1 subpass, which is the minimum you can do
	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	vk::SubpassDependency color_dependency = {};
	color_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	color_dependency.dstSubpass = 0;
	color_dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	color_dependency.srcAccessMask = {};
	color_dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	color_dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

	vk::SubpassDependency depth_dependency = {};
	depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depth_dependency.dstSubpass = 0;
	depth_dependency.srcStageMask =
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
	depth_dependency.srcAccessMask = {};
	depth_dependency.dstStageMask =
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
	depth_dependency.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	vk::AttachmentDescription attachments[2] = { color_attachment, depth_attachment };
	vk::SubpassDependency dependencies[2] = { color_dependency, depth_dependency };

	vk::RenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = vk::StructureType::eRenderPassCreateInfo;
	//connect the color and depth attachments to the info
	render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = &attachments[0];
	//connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	// connect the dependencies to the info
	render_pass_info.dependencyCount = 2;
	render_pass_info.pDependencies = &dependencies[0];

	VK_CHECK(device.createRenderPass(&render_pass_info, nullptr, &render_pass));

	main_deletion_queue.push_function([=, this]() { device.destroyRenderPass(render_pass); });
}

void RenderManager::init_framebuffers() {
	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for
	//rendering
	vk::FramebufferCreateInfo fb_info = {};
	fb_info.sType = vk::StructureType::eFramebufferCreateInfo;
	fb_info.pNext = nullptr;

	fb_info.renderPass = render_pass;
	fb_info.attachmentCount = 1;
	fb_info.width = window_extent.width;
	fb_info.height = window_extent.height;
	fb_info.layers = 1;

	//grab how many images we have in the swapchain
	const uint32_t swapchain_imagecount = swapchain_images.size();
	framebuffers = std::vector<vk::Framebuffer>(swapchain_imagecount);

	//create framebuffers for each of the swapchain image views
	for (int i = 0; i < swapchain_imagecount; i++) {
		// We're reusing the same depth image view for all framebuffers, since we're only drawing one frame at a
		// time
		vk::ImageView attachments[2] = { swapchain_image_views[i], depth_image_view };

		fb_info.pAttachments = &attachments[0];
		fb_info.attachmentCount = 2;
		VK_CHECK(device.createFramebuffer(&fb_info, nullptr, &framebuffers[i]));

		main_deletion_queue.push_function([=, this]() {
			device.destroyFramebuffer(framebuffers[i]);
			device.destroyImageView(swapchain_image_views[i]);
		});
	}
}

void RenderManager::init_sync_structures() {
	//create synchronization structures
	vk::FenceCreateInfo fence_create_info = vk_init::fence_create_info(vk::FenceCreateFlagBits::eSignaled);

	for (auto &frame : frames) {
		VK_CHECK(device.createFence(&fence_create_info, nullptr, &frame.render_fence));

		main_deletion_queue.push_function([=, this]() { device.destroyFence(frame.render_fence); });

		//for the semaphores we don't need any flags
		vk::SemaphoreCreateInfo semaphore_create_info = vk_init::semaphore_create_info();

		VK_CHECK(device.createSemaphore(&semaphore_create_info, nullptr, &frame.present_semaphore));
		VK_CHECK(device.createSemaphore(&semaphore_create_info, nullptr, &frame.render_semaphore));

		main_deletion_queue.push_function([=, this]() {
			device.destroySemaphore(frame.present_semaphore);
			device.destroySemaphore(frame.render_semaphore);
		});
	}

	vk::FenceCreateInfo upload_fence_create_info = vk_init::fence_create_info();

	VK_CHECK(device.createFence(&upload_fence_create_info, nullptr, &upload_context.upload_fence));

	main_deletion_queue.push_function([=, this]() { device.destroyFence(upload_context.upload_fence); });
}

void RenderManager::init_descriptors() {
	const size_t scene_param_buffer_size = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));

	scene_parameter_buffer = create_buffer(
			scene_param_buffer_size, vk::BufferUsageFlagBits::eUniformBuffer, vma::MemoryUsage::eCpuToGpu);

	//information about the binding.
	vk::DescriptorSetLayoutBinding camera_bind = vk_init::descriptor_set_layout_binding(
			vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 0);

	//binding for scene data at 1
	vk::DescriptorSetLayoutBinding scene_bind =
			vk_init::descriptor_set_layout_binding(vk::DescriptorType::eUniformBufferDynamic,
					vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 1);

	vk::DescriptorSetLayoutBinding bindings[] = { camera_bind, scene_bind };

	vk::DescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
	setinfo.pNext = nullptr;

	setinfo.bindingCount = 2;
	//no flags
	setinfo.flags = {};
	//point to the camera buffer binding
	setinfo.pBindings = bindings;

	VK_CHECK(device.createDescriptorSetLayout(&setinfo, nullptr, &global_set_layout));

	//create a descriptor pool that will hold 10 uniform buffers
	std::vector<vk::DescriptorPoolSize> sizes = { { vk::DescriptorType::eUniformBuffer, 10 },
		{ vk::DescriptorType::eUniformBufferDynamic, 10 }, { vk::DescriptorType::eStorageBuffer, 10 },
		{ vk::DescriptorType::eCombinedImageSampler, 10 } };

	vk::DescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = vk::StructureType::eDescriptorPoolCreateInfo;
	pool_info.flags = {};
	pool_info.maxSets = 10;
	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes = sizes.data();

	VK_CHECK(device.createDescriptorPool(&pool_info, nullptr, &descriptor_pool));

	vk::DescriptorSetLayoutBinding object_bind = vk_init::descriptor_set_layout_binding(
			vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex, 0);
	vk::DescriptorSetLayoutCreateInfo object_set_info = {};
	object_set_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
	object_set_info.pNext = nullptr;
	object_set_info.bindingCount = 1;
	object_set_info.flags = {};
	object_set_info.pBindings = &object_bind;

	VK_CHECK(device.createDescriptorSetLayout(&object_set_info, nullptr, &object_set_layout));

	vk::DescriptorSetLayoutBinding texture_bind = vk_init::descriptor_set_layout_binding(
			vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 0);

	vk::DescriptorSetLayoutCreateInfo set3_info = {};
	set3_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
	set3_info.pNext = nullptr;
	set3_info.bindingCount = 1;
	set3_info.flags = {};
	set3_info.pBindings = &texture_bind;

	VK_CHECK(device.createDescriptorSetLayout(&set3_info, nullptr, &single_texture_set_layout));

	for (auto &frame : frames) {
		frame.camera_buffer = create_buffer(
				sizeof(GPUCameraData), vk::BufferUsageFlagBits::eUniformBuffer, vma::MemoryUsage::eCpuToGpu);

		// TODO: Move this constant somewhere else
		const int max_objects = 10000;
		frame.object_buffer = create_buffer(max_objects * sizeof(GPUObjectData),
				vk::BufferUsageFlagBits::eStorageBuffer, vma::MemoryUsage::eCpuToGpu);

		//allocate one descriptor set for each frames[i]
		vk::DescriptorSetAllocateInfo alloc_info = {};
		alloc_info.pNext = nullptr;
		alloc_info.sType = vk::StructureType::eDescriptorSetAllocateInfo;
		//using the pool we just set
		alloc_info.descriptorPool = descriptor_pool;
		//only 1 descriptor
		alloc_info.descriptorSetCount = 1;
		//using the global data layout
		alloc_info.pSetLayouts = &global_set_layout;

		// allocate the descriptor set
		VK_CHECK(device.allocateDescriptorSets(&alloc_info, &frame.global_descriptor));

		//allocate the descriptor set that will point to object buffer
		vk::DescriptorSetAllocateInfo object_set_alloc = {};
		object_set_alloc.pNext = nullptr;
		object_set_alloc.sType = vk::StructureType::eDescriptorSetAllocateInfo;
		object_set_alloc.descriptorPool = descriptor_pool;
		object_set_alloc.descriptorSetCount = 1;
		object_set_alloc.pSetLayouts = &object_set_layout;

		VK_CHECK(device.allocateDescriptorSets(&object_set_alloc, &frame.object_descriptor));

		//information about the buffer we want to point at in the descriptor
		vk::DescriptorBufferInfo camera_info;
		//it will be the camera buffer
		camera_info.buffer = frame.camera_buffer.buffer;
		//at 0 offset
		camera_info.offset = 0;
		//of the size of a camera data struct
		camera_info.range = sizeof(GPUCameraData);

		vk::DescriptorBufferInfo scene_info;
		scene_info.buffer = scene_parameter_buffer.buffer;
		scene_info.offset = 0;
		scene_info.range = sizeof(GPUSceneData);

		vk::DescriptorBufferInfo object_buffer_info;
		object_buffer_info.buffer = frame.object_buffer.buffer;
		object_buffer_info.offset = 0;
		object_buffer_info.range = sizeof(GPUObjectData) * max_objects;

		vk::WriteDescriptorSet camera_write = vk_init::write_descriptor_buffer(
				vk::DescriptorType::eUniformBuffer, frame.global_descriptor, &camera_info, 0);
		vk::WriteDescriptorSet scene_write = vk_init::write_descriptor_buffer(
				vk::DescriptorType::eUniformBufferDynamic, frame.global_descriptor, &scene_info, 1);
		VkWriteDescriptorSet object_write = vk_init::write_descriptor_buffer(
				vk::DescriptorType::eStorageBuffer, frame.object_descriptor, &object_buffer_info, 0);

		vk::WriteDescriptorSet set_writes[] = { camera_write, scene_write, object_write };

		//update the descriptor set
		device.updateDescriptorSets(3, set_writes, 0, nullptr);
	}

	main_deletion_queue.push_function([&]() {
		device.destroyDescriptorSetLayout(global_set_layout, nullptr);
		device.destroyDescriptorPool(descriptor_pool, nullptr);
		allocator.destroyBuffer(scene_parameter_buffer.buffer, scene_parameter_buffer.allocation);

		for (auto &frame : frames) {
			allocator.destroyBuffer(frame.camera_buffer.buffer, frame.camera_buffer.allocation);
			allocator.destroyBuffer(frame.object_buffer.buffer, frame.object_buffer.allocation);
		}

		device.destroyDescriptorSetLayout(object_set_layout, nullptr);
		device.destroyDescriptorSetLayout(single_texture_set_layout, nullptr);
	});
}

void RenderManager::init_pipelines() {
	vk::ShaderModule mesh_frag_shader;
	if (!load_shader_module("resources/shaders/default_lit.frag.spv", &mesh_frag_shader)) {
		SPDLOG_ERROR("Error when building the triangle fragment shader module");
	} else {
		SPDLOG_INFO("Triangle fragment shader successfully loaded");
	}

	vk::ShaderModule textured_mesh_frag_shader;
	if (!load_shader_module("resources/shaders/textured_lit.frag.spv", &textured_mesh_frag_shader)) {
		SPDLOG_ERROR("Error when building the textured triangle fragment shader module");
	} else {
		SPDLOG_INFO("Textured triangle fragment shader successfully loaded");
	}

	vk::ShaderModule mesh_vertex_shader;
	if (!load_shader_module("resources/shaders/tri_mesh.vert.spv", &mesh_vertex_shader)) {
		SPDLOG_ERROR("Error when building the mesh vertex shader module");
		abort();
	} else {
		SPDLOG_INFO("Triangle vertex shader successfully loaded");
	}

	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader
	//modules per stage
	PipelineBuilder pipeline_builder;

	//vertex input controls how to read vertices from vertex buffers. We aren't using it yet
	pipeline_builder.vertex_input_info = vk_init::vertex_input_state_create_info();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw_objects triangle list
	pipeline_builder.input_assembly = vk_init::input_assembly_create_info(vk::PrimitiveTopology::eTriangleList);

	//build viewport and scissor from the swapchain extents

	// We flip the viewport's y-axis to match other's APIs coordinate system
	// https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
	pipeline_builder.viewport.x = 0.0f;
	pipeline_builder.viewport.y = (float)window_extent.height;
	pipeline_builder.viewport.width = (float)window_extent.width;
	pipeline_builder.viewport.height = -(float)window_extent.height;
	pipeline_builder.viewport.minDepth = 0.0f;
	pipeline_builder.viewport.maxDepth = 1.0f;

	pipeline_builder.scissor.offset = vk::Offset2D(0, 0);
	pipeline_builder.scissor.extent = window_extent;

	//configure the rasterizer to draw_objects filled triangles
	pipeline_builder.rasterizer = vk_init::rasterization_state_create_info(vk::PolygonMode::eFill);

	//we don't use multisampling, so just run the default one
	pipeline_builder.multisampling = vk_init::multisampling_state_create_info();

	//a single blend attachment with no blending and writing to RGBA
	pipeline_builder.color_blend_attachment = vk_init::color_blend_attachment_state();

	//default depthtesting
	pipeline_builder.depth_stencil = vk_init::depth_stencil_create_info(true, true, vk::CompareOp::eLessOrEqual);

	//build the mesh pipeline
	//we start from just the default empty pipeline layout info
	vk::PipelineLayoutCreateInfo mesh_pipeline_layout_info = vk_init::pipeline_layout_create_info();

	//setup push constants
	vk::PushConstantRange push_constant;
	//this push constant range starts at the beginning
	push_constant.offset = 0;
	//this push constant range takes up the size of a MeshPushConstants struct
	push_constant.size = sizeof(MeshPushConstants);
	//this push constant range is accessible only in the vertex shader
	push_constant.stageFlags = vk::ShaderStageFlagBits::eVertex;

	//push-constant setup
	mesh_pipeline_layout_info.pPushConstantRanges = &push_constant;
	mesh_pipeline_layout_info.pushConstantRangeCount = 1;

	//hook the global set layout
	vk::DescriptorSetLayout set_layouts[] = { global_set_layout, object_set_layout };

	mesh_pipeline_layout_info.setLayoutCount = 2;
	mesh_pipeline_layout_info.pSetLayouts = set_layouts;

	VK_CHECK(device.createPipelineLayout(&mesh_pipeline_layout_info, nullptr, &mesh_pipeline_layout));

	pipeline_builder.pipeline_layout = mesh_pipeline_layout;

	VertexInputDescription vertex_description = Vertex::get_vertex_description();

	//connect the pipeline builder vertex input info to the one we get from Vertex
	pipeline_builder.vertex_input_info.pVertexAttributeDescriptions = vertex_description.attributes.data();
	pipeline_builder.vertex_input_info.vertexAttributeDescriptionCount = vertex_description.attributes.size();

	pipeline_builder.vertex_input_info.pVertexBindingDescriptions = vertex_description.bindings.data();
	pipeline_builder.vertex_input_info.vertexBindingDescriptionCount = vertex_description.bindings.size();

	//add the other shaders
	pipeline_builder.shader_stages.push_back(
			vk_init::pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eVertex, mesh_vertex_shader));

	//make sure that triangle_frag_shader is holding the compiled default_lit.frag
	pipeline_builder.shader_stages.push_back(
			vk_init::pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eFragment, mesh_frag_shader));

	//build the mesh triangle pipeline
	mesh_pipeline = pipeline_builder.build_pipeline(device, render_pass);

	create_material(mesh_pipeline, mesh_pipeline_layout, "default_mesh");

	// create pipeline for textured rendering
	vk::PipelineLayoutCreateInfo textured_mesh_pipeline_layout_info = mesh_pipeline_layout_info;
	vk::DescriptorSetLayout set_layouts2[] = { global_set_layout, object_set_layout, single_texture_set_layout };
	textured_mesh_pipeline_layout_info.setLayoutCount = 3;
	textured_mesh_pipeline_layout_info.pSetLayouts = set_layouts2;

	vk::PipelineLayout textured_mesh_pipeline_layout;
	VK_CHECK(device.createPipelineLayout(&textured_mesh_pipeline_layout_info, nullptr, &textured_mesh_pipeline_layout));

	pipeline_builder.shader_stages.clear();
	pipeline_builder.shader_stages.push_back(
			vk_init::pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eVertex, mesh_vertex_shader));

	//make sure that triangle_frag_shader is holding the compiled default_lit.frag
	pipeline_builder.shader_stages.push_back(
			vk_init::pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eFragment, textured_mesh_frag_shader));

	pipeline_builder.pipeline_layout = textured_mesh_pipeline_layout;
	vk::Pipeline textured_mesh_pipeline = pipeline_builder.build_pipeline(device, render_pass);
	create_material(textured_mesh_pipeline, textured_mesh_pipeline_layout, "textured_mesh");

	device.destroyShaderModule(mesh_vertex_shader, nullptr);
	device.destroyShaderModule(mesh_frag_shader, nullptr);
	device.destroyShaderModule(textured_mesh_frag_shader, nullptr);

	main_deletion_queue.push_function([=, this]() {
		device.destroyPipeline(mesh_pipeline);
		device.destroyPipelineLayout(mesh_pipeline_layout);
		device.destroyPipeline(textured_mesh_pipeline);
		device.destroyPipelineLayout(textured_mesh_pipeline_layout);
	});
}

void RenderManager::init_scene() {
	RenderObject box = {};
	box.mesh = get_mesh("box");
	box.material = get_material("textured_mesh");
	glm::mat4 rotation = glm::rotate(glm::mat4{ 1.0 }, glm::radians(45.0f), glm::vec3(0, 1, 0));
	box.transform_matrix = glm::mat4{ 1.0f } * rotation;

	renderables.push_back(box);

	for (int x = -20; x <= 20; x++) {
		for (int y = -20; y <= 20; y++) {
			RenderObject tri = {};
			tri.mesh = get_mesh("triangle");
			tri.material = get_material("default_mesh");
			glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(x, 0, y));
			glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(0.2, 0.2, 0.2));
			tri.transform_matrix = translation * scale;

			renderables.push_back(tri);
		}
	}

	//create a sampler for the texture
	vk::SamplerCreateInfo sampler_info = vk_init::sampler_create_info(vk::Filter::eNearest);

	vk::Sampler blocky_sampler;
	VK_CHECK(device.createSampler(&sampler_info, nullptr, &blocky_sampler));

	main_deletion_queue.push_function([=, this]() { device.destroySampler(blocky_sampler); });

	Material *textured_mat = get_material("textured_mesh");

	//allocate the descriptor set for single-texture to use on the material
	vk::DescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = vk::StructureType::eDescriptorSetAllocateInfo;
	alloc_info.pNext = nullptr;
	alloc_info.descriptorPool = descriptor_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &single_texture_set_layout;

	VK_CHECK(device.allocateDescriptorSets(&alloc_info, &textured_mat->texture_set));

	//write to the descriptor set so that it points to our empire_diffuse texture
	vk::DescriptorImageInfo image_buffer_info;
	image_buffer_info.sampler = blocky_sampler;
	image_buffer_info.imageView = loaded_textures["empire_diffuse"].image_view;
	image_buffer_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::WriteDescriptorSet texture1 = vk_init::write_descriptor_image(
			vk::DescriptorType::eCombinedImageSampler, textured_mat->texture_set, &image_buffer_info, 0);

	device.updateDescriptorSets(1, &texture1, 0, nullptr);
}

void RenderManager::init_imgui(GLFWwindow *window) {
	//1: create descriptor pool for IMGUI
	// the size of the pool is very oversize, but it's copied from imgui demo itself.
	vk::DescriptorPoolSize pool_sizes[] = { { vk::DescriptorType::eSampler, 1000 },
		{ vk::DescriptorType::eCombinedImageSampler, 1000 }, { vk::DescriptorType::eSampledImage, 1000 },
		{ vk::DescriptorType::eStorageImage, 1000 }, { vk::DescriptorType::eUniformTexelBuffer, 1000 },
		{ vk::DescriptorType::eStorageTexelBuffer, 1000 }, { vk::DescriptorType::eUniformBuffer, 1000 },
		{ vk::DescriptorType::eStorageBuffer, 1000 }, { vk::DescriptorType::eUniformBufferDynamic, 1000 },
		{ vk::DescriptorType::eStorageBufferDynamic, 1000 }, { vk::DescriptorType::eInputAttachment, 1000 } };

	vk::DescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = vk::StructureType::eDescriptorPoolCreateInfo;
	pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	vk::DescriptorPool imgui_pool;
	VK_CHECK(device.createDescriptorPool(&pool_info, nullptr, &imgui_pool));

	// 2: initialize imgui library

	//this initializes the core structures of imgui
	ImGui::CreateContext();

	//this initializes imgui for GLFW
	ImGui_ImplGlfw_InitForVulkan(window, true);

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
	init_info.PhysicalDevice = chosen_gpu;
	init_info.Device = device;
	init_info.Queue = graphics_queue;
	init_info.DescriptorPool = imgui_pool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, render_pass);

	//execute a gpu command to upload imgui font textures
	immediate_submit([&](vk::CommandBuffer cmd) { ImGui_ImplVulkan_CreateFontsTexture(cmd); });

	//clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();

	//queue destruction of the imgui created structures
	main_deletion_queue.push_function([=, this]() {
		device.destroyDescriptorPool(imgui_pool, nullptr);
		ImGui_ImplVulkan_Shutdown();
	});

	// zajebisty styl wulkanowy czerwony 😎
	// TODO: zrobic styl wulkanowy fajniejszy
	//	ImGuiStyle &vulkan_style = ImGui::GetStyle();
	//	vulkan_style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	//	vulkan_style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	//	vulkan_style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	//	vulkan_style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	//	vulkan_style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
}

bool RenderManager::load_shader_module(const char *file_path, vk::ShaderModule *out_shader_module) {
	std::ifstream file(file_path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return false;
	}

	//find what the size of the file is by looking up the location of the cursor
	//because the cursor is at the end, it gives the size directly in bytes
	size_t file_size = (size_t)file.tellg();

	//spirv expects the buffer to be on uint32, so make sure to reserve an int vector big
	//enough for the entire file
	std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

	//put file cursor at beginning
	file.seekg(0);

	//load the entire file into the buffer
	file.read((char *)buffer.data(), (std::streamsize)file_size);

	//now that the file is loaded into the buffer, we can close it
	file.close();

	//create a new shader module, using the buffer we loaded
	vk::ShaderModuleCreateInfo create_info = {};
	create_info.sType = vk::StructureType::eShaderModuleCreateInfo;
	create_info.pNext = nullptr;

	//codeSize has to be in bytes, so multiply the ints in the buffer by size
	//of int to know the real size of the buffer
	create_info.codeSize = buffer.size() * sizeof(uint32_t);
	create_info.pCode = buffer.data();

	//check that the creation goes well.
	vk::ShaderModule shader_module;
	if (device.createShaderModule(&create_info, nullptr, &shader_module) != vk::Result::eSuccess) {
		return false;
	}
	*out_shader_module = shader_module;

	return true;
}

void RenderManager::load_meshes() {
	triangle_mesh.vertices.resize(3);

	//vertex positions
	triangle_mesh.vertices[0].position = { 1.f, 1.f, 0.0f };
	triangle_mesh.vertices[1].position = { -1.f, 1.f, 0.0f };
	triangle_mesh.vertices[2].position = { 0.f, -1.f, 0.0f };

	//vertex colors, all green
	triangle_mesh.vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
	triangle_mesh.vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	triangle_mesh.vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green

	triangle_mesh.indices = { 0, 1, 2 };

	//we don't care about the vertex normals

	box_mesh.load_from_gltf("resources/models/BoxTextured.gltf");

	upload_mesh(triangle_mesh);
	upload_mesh(box_mesh);

	meshes["triangle"] = triangle_mesh;
	meshes["box"] = box_mesh;
}

void RenderManager::upload_mesh(Mesh &mesh) {
	size_t buffer_size = mesh.vertices.size() * sizeof(Vertex);
	//allocate staging buffer
	vk::BufferCreateInfo staging_buffer_info = {};
	staging_buffer_info.sType = vk::StructureType::eBufferCreateInfo;
	staging_buffer_info.pNext = nullptr;

	staging_buffer_info.size = buffer_size;
	staging_buffer_info.usage = vk::BufferUsageFlagBits::eTransferSrc;

	//let the VMA library know that this data should be on CPU RAM
	vma::AllocationCreateInfo vmaalloc_info = {};
	vmaalloc_info.usage = vma::MemoryUsage::eCpuOnly;

	AllocatedBuffer staging_buffer;

	//allocate the buffer
	VK_CHECK(allocator.createBuffer(
			&staging_buffer_info, &vmaalloc_info, &staging_buffer.buffer, &staging_buffer.allocation, nullptr));

	void *data;
	VK_CHECK(allocator.mapMemory(staging_buffer.allocation, &data));
	memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
	allocator.unmapMemory(staging_buffer.allocation);

	//allocate vertex buffer
	vk::BufferCreateInfo buffer_info = {};
	buffer_info.sType = vk::StructureType::eBufferCreateInfo;
	//this is the total size, in bytes, of the buffer we are allocating
	buffer_info.size = mesh.vertices.size() * sizeof(Vertex);
	//this buffer is going to be used as a Vertex Buffer
	buffer_info.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;

	//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
	vmaalloc_info.usage = vma::MemoryUsage::eGpuOnly;

	//allocate the buffer
	VK_CHECK(allocator.createBuffer(
			&buffer_info, &vmaalloc_info, &mesh.vertex_buffer.buffer, &mesh.vertex_buffer.allocation, nullptr));

	//add the destruction of triangle mesh buffer to the deletion queue
	main_deletion_queue.push_function(
			[=, this]() { allocator.destroyBuffer(mesh.vertex_buffer.buffer, mesh.vertex_buffer.allocation); });

	// copy vertex data
	immediate_submit([=](vk::CommandBuffer cmd) {
		vk::BufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = buffer_size;
		cmd.copyBuffer(staging_buffer.buffer, mesh.vertex_buffer.buffer, 1, &copy);
	});

	allocator.destroyBuffer(staging_buffer.buffer, staging_buffer.allocation);

	// Recreate staging buffer, this time for indices
	buffer_size = mesh.indices.size() * sizeof(uint32_t);
	staging_buffer_info.size = buffer_size;
	vmaalloc_info.usage = vma::MemoryUsage::eCpuOnly;

	VK_CHECK(allocator.createBuffer(
			&staging_buffer_info, &vmaalloc_info, &staging_buffer.buffer, &staging_buffer.allocation, nullptr));

	VK_CHECK(allocator.mapMemory(staging_buffer.allocation, &data));
	memcpy(data, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));
	allocator.unmapMemory(staging_buffer.allocation);

	//allocate index buffer
	vmaalloc_info.usage = vma::MemoryUsage::eGpuOnly;

	buffer_info.size = buffer_size;
	buffer_info.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	VK_CHECK(allocator.createBuffer(
			&buffer_info, &vmaalloc_info, &mesh.index_buffer.buffer, &mesh.index_buffer.allocation, nullptr));

	//add the destruction of triangle mesh buffer to the deletion queue
	main_deletion_queue.push_function(
			[=, this]() { allocator.destroyBuffer(mesh.index_buffer.buffer, mesh.index_buffer.allocation); });

	//copy index data
	immediate_submit([=](vk::CommandBuffer cmd) {
		vk::BufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = buffer_size;
		cmd.copyBuffer(staging_buffer.buffer, mesh.index_buffer.buffer, 1, &copy);
	});

	allocator.destroyBuffer(staging_buffer.buffer, staging_buffer.allocation);
}

AllocatedBuffer RenderManager::create_buffer(
		size_t alloc_size, vk::BufferUsageFlags usage, vma::MemoryUsage memory_usage) const {
	//allocate vertex buffer
	vk::BufferCreateInfo buffer_info = {};
	buffer_info.sType = vk::StructureType::eBufferCreateInfo;
	buffer_info.pNext = nullptr;

	buffer_info.size = alloc_size;
	buffer_info.usage = usage;

	vma::AllocationCreateInfo vmaalloc_info = {};
	vmaalloc_info.usage = memory_usage;

	AllocatedBuffer new_buffer;

	//allocate the buffer
	VK_CHECK(allocator.createBuffer(&buffer_info, &vmaalloc_info, &new_buffer.buffer, &new_buffer.allocation, nullptr));

	return new_buffer;
}

size_t RenderManager::pad_uniform_buffer_size(size_t original_size) const {
	// Calculate required alignment based on minimum device offset alignment
	size_t min_ubo_alignment = gpu_properties.limits.minUniformBufferOffsetAlignment;
	size_t aligned_size = original_size;
	if (min_ubo_alignment > 0) {
		aligned_size = (aligned_size + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);
	}
	return aligned_size;
}

void RenderManager::immediate_submit(std::function<void(vk::CommandBuffer)> &&function) {
	vk::CommandBuffer cmd = upload_context.command_buffer;

	//begin the command buffer recording. We will use this command buffer exactly once before resetting, so we tell
	//vulkan that
	vk::CommandBufferBeginInfo cmd_begin_info =
			vk_init::command_buffer_begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	VK_CHECK(cmd.begin(&cmd_begin_info));

	//execute the function
	function(cmd);

	VK_CHECK(cmd.end());

	vk::SubmitInfo submit = vk_init::submit_info(&cmd);

	//submit command buffer to the queue and execute it.
	// _uploadFence will now block until the graphic commands finish execution
	VK_CHECK(graphics_queue.submit(1, &submit, upload_context.upload_fence));

	VK_CHECK(device.waitForFences(1, &upload_context.upload_fence, true, 9999999999));
	VK_CHECK(device.resetFences(1, &upload_context.upload_fence));

	// reset the command buffers inside the command pool
	device.resetCommandPool(upload_context.command_pool);
}

RenderManager::Status RenderManager::startup(DisplayManager &display_manager) {
	// TODO: Handle failures
	init_vulkan(display_manager);
	init_swapchain(display_manager);
	init_commands();
	init_default_renderpass();
	init_framebuffers();
	init_sync_structures();
	init_descriptors();
	init_pipelines();
	load_meshes();
	load_images();
	init_scene();

	init_imgui(display_manager.window);

	return Status::Ok;
}

void RenderManager::shutdown() {
	//make sure the GPU has stopped doing its things
	VK_CHECK(device.waitIdle());

	main_deletion_queue.flush();

	device.destroy();
	instance.destroySurfaceKHR(surface);
	vkb::destroy_debug_utils_messenger(instance, debug_messenger);
	instance.destroy();
}

FrameData &RenderManager::get_current_frame() {
	return frames[frame_number % FRAME_OVERLAP];
}

Material *RenderManager::create_material(vk::Pipeline pipeline, vk::PipelineLayout layout, const std::string &name) {
	Material mat = {};
	mat.pipeline = pipeline;
	mat.pipeline_layout = layout;
	materials[name] = mat;
	return &materials[name];
}

Material *RenderManager::get_material(const std::string &name) {
	//search for the object, and return nullptr if not found
	auto it = materials.find(name);
	if (it == materials.end()) {
		return nullptr;
	} else {
		return &(*it).second;
	}
}

Mesh *RenderManager::get_mesh(const std::string &name) {
	auto it = meshes.find(name);
	if (it == meshes.end()) {
		return nullptr;
	} else {
		return &(*it).second;
	}
}

void RenderManager::load_images() {
	Texture texture = {};

	vk_util::load_image_from_file(*this, "resources/models/CesiumLogoFlat.png", texture.image);

	vk::ImageViewCreateInfo image_info = vk_init::image_view_create_info(
			vk::Format::eR8G8B8A8Srgb, texture.image.image, vk::ImageAspectFlagBits::eColor);

	VK_CHECK(device.createImageView(&image_info, nullptr, &texture.image_view));

	main_deletion_queue.push_function([=, this]() { device.destroyImageView(texture.image_view); });

	loaded_textures["empire_diffuse"] = texture;
}

void RenderManager::draw() {
	//wait until the GPU has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(device.waitForFences(1, &get_current_frame().render_fence, true, 1000000000));
	VK_CHECK(device.resetFences(1, &get_current_frame().render_fence));

	//request image from the swapchain, one second timeout
	uint32_t swapchain_image_index;
	VK_CHECK(device.acquireNextImageKHR(
			swapchain, 1000000000, get_current_frame().present_semaphore, nullptr, &swapchain_image_index));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin
	//recording again.
	VK_CHECK(get_current_frame().main_command_buffer.reset());

	//naming it cmd for shorter writing
	vk::CommandBuffer cmd = get_current_frame().main_command_buffer;

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan
	//know that
	vk::CommandBufferBeginInfo cmd_begin_info = {};
	cmd_begin_info.sType = vk::StructureType::eCommandBufferBeginInfo;
	cmd_begin_info.pNext = nullptr;

	cmd_begin_info.pInheritanceInfo = nullptr;
	cmd_begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	VK_CHECK(cmd.begin(&cmd_begin_info));
	//make a clear-color from frame number. This will flash with a 120*pi frame period.
	vk::ClearValue color_clear_value;
	//	float flash = abs(sin((float)frame_number / 15.f));
	//	color_clear_value.color.setFloat32({ 0.0f, 0.0f, flash, 1.0f });
	color_clear_value.color.setFloat32({ 0.01f, 0.01f, 0.01f, 1.0f });

	vk::ClearValue depth_clear_value;
	depth_clear_value.depthStencil.setDepth(1.0f);

	vk::ClearValue clear_values[] = { color_clear_value, depth_clear_value };

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	vk::RenderPassBeginInfo rp_info = {};
	rp_info.sType = vk::StructureType::eRenderPassBeginInfo;
	rp_info.pNext = nullptr;

	// connect clear values

	rp_info.renderPass = render_pass;
	rp_info.renderArea.offset.x = 0;
	rp_info.renderArea.offset.y = 0;
	rp_info.renderArea.extent = window_extent;
	rp_info.framebuffer = framebuffers[swapchain_image_index];

	//connect clear values
	rp_info.clearValueCount = 2;
	rp_info.pClearValues = &clear_values[0];

	cmd.beginRenderPass(&rp_info, vk::SubpassContents::eInline);

	draw_objects(cmd, renderables.data(), (int)renderables.size());

	ImGui::Render();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	cmd.endRenderPass();
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(cmd.end());

	//prepare the submission to the queue.
	//we want to wait on the _present_semaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the render_semaphore, to signal that rendering has finished

	vk::SubmitInfo submit = {};
	submit.sType = vk::StructureType::eSubmitInfo;
	submit.pNext = nullptr;

	vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	submit.pWaitDstStageMask = &wait_stage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &get_current_frame().present_semaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &get_current_frame().render_semaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;

	//submit command buffer to the queue and execute it.
	// render_fence will now block until the graphic commands finish execution
	VK_CHECK(graphics_queue.submit(1, &submit, get_current_frame().render_fence));

	// this will put the image we just rendered into the visible window.
	// we want to wait on the render_semaphore for that,
	// as it's necessary that drawing commands have finished before the image is displayed to the user
	vk::PresentInfoKHR present_info = {};
	present_info.sType = vk::StructureType::ePresentInfoKHR;
	present_info.pNext = nullptr;

	present_info.pSwapchains = &swapchain;
	present_info.swapchainCount = 1;

	present_info.pWaitSemaphores = &get_current_frame().render_semaphore;
	present_info.waitSemaphoreCount = 1;

	present_info.pImageIndices = &swapchain_image_index;

	VK_CHECK(graphics_queue.presentKHR(&present_info));

	frame_number++;
}

void RenderManager::draw_objects(vk::CommandBuffer cmd, RenderObject *first, int count) {
	// TODO: Sort RenderObjects by material and mesh to reduce pipeline and descriptor set changes

	//make a model view matrix for rendering the object
	//camera view
	static glm::vec3 cam_pos = { 0.f, 0.f, -25.f };

	static float fov = 60.f;

	static float pitch = 0.f;
	static float yaw = 0.f;

	glm::vec3 final_cam_pos = cam_pos;
	final_cam_pos.y *= -1.f;

	glm::mat4 view = glm::translate(glm::mat4(1.f), final_cam_pos) *
			glm::rotate(glm::mat4(1.f), pitch, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1.f), yaw, glm::vec3(0, 1, 0));
	//camera projection
	static float aspect = (float)window_extent.width / (float)window_extent.height;
	glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 200.0f);

	float pi = 3.14f;

	ImGui::Begin("Camera");
	ImGui::SliderFloat("Y", &cam_pos.y, -50, 50);
	ImGui::SliderFloat("Z", &cam_pos.z, -100, -2);
	ImGui::SliderFloat("Pitch", &pitch, -pi, pi);
	ImGui::SliderFloat("Yaw", &yaw, -pi, pi);
	ImGui::SliderFloat("FOV", &fov, 30, 120);
	ImGui::End();

	//fill a GPU camera data struct
	GPUCameraData cam_data = {};
	cam_data.proj = projection;
	cam_data.view = view;
	cam_data.viewproj = projection * view;

	//and copy it to the buffer
	void *data;
	VK_CHECK(allocator.mapMemory(get_current_frame().camera_buffer.allocation, &data));
	memcpy(data, &cam_data, sizeof(GPUCameraData));
	allocator.unmapMemory(get_current_frame().camera_buffer.allocation);

	float framed = ((float)frame_number / 30.f);

	scene_parameters.ambient_color = { sin(framed), 0, cos(framed), 1 };

	char *scene_data;
	VK_CHECK(allocator.mapMemory(scene_parameter_buffer.allocation, (void **)&scene_data));
	unsigned int frame_index = frame_number % FRAME_OVERLAP;
	scene_data += pad_uniform_buffer_size(sizeof(GPUSceneData)) * frame_index;
	memcpy(scene_data, &scene_parameters, sizeof(GPUSceneData));
	allocator.unmapMemory(scene_parameter_buffer.allocation);

	void *object_data;
	VK_CHECK(allocator.mapMemory(get_current_frame().object_buffer.allocation, &object_data));
	auto *gpu_object_data = (GPUObjectData *)object_data;
	for (int i = 0; i < count; i++) {
		RenderObject &object = first[i];
		//fill a GPU object data struct
		gpu_object_data[i].model_matrix = object.transform_matrix;
	}
	allocator.unmapMemory(get_current_frame().object_buffer.allocation);

	Mesh *last_mesh = nullptr;
	Material *last_material = nullptr;
	for (int i = 0; i < count; i++) {
		RenderObject &object = first[i];

		//only bind the pipeline if it doesn't match with the already bound one
		if (object.material != last_material) {
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, object.material->pipeline);
			last_material = object.material;

			uint32_t uniform_offset = pad_uniform_buffer_size(sizeof(GPUSceneData)) * frame_index;

			// bind the descriptor set when changing pipeline
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, object.material->pipeline_layout, 0, 1,
					&get_current_frame().global_descriptor, 1, &uniform_offset);

			// bind the object descriptor set
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, object.material->pipeline_layout, 1, 1,
					&get_current_frame().object_descriptor, 0, nullptr);

			if (object.material->texture_set != (vk::DescriptorSet)VK_NULL_HANDLE) {
				//texture descriptor
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, object.material->pipeline_layout, 2, 1,
						&object.material->texture_set, 0, nullptr);
			}
		}

		glm::mat4 model = object.transform_matrix;
		//final render matrix, that we are calculating on the cpu
		glm::mat4 mesh_matrix = projection * view * model;

		MeshPushConstants constants = {};
		constants.render_matrix = object.transform_matrix;

		//upload the mesh to the GPU via push constants
		cmd.pushConstants(object.material->pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0,
				sizeof(MeshPushConstants), &constants);

		//only bind the mesh if it's a different one from last bind
		if (object.mesh != last_mesh) {
			//bind the mesh vertex buffer with offset 0
			vk::DeviceSize offset = 0;
			cmd.bindVertexBuffers(0, 1, &object.mesh->vertex_buffer.buffer, &offset);
			cmd.bindIndexBuffer(object.mesh->index_buffer.buffer, 0, vk::IndexType::eUint32);
			last_mesh = object.mesh;
		}
		//we can now draw
		// We're passing object's index as instanceCount, to easily pass integer to the shader
		cmd.drawIndexed(object.mesh->indices.size(), 1, 0, 0, i);
	}
}