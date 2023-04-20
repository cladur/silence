#include "render_manager.h"
#include "asset_loader.h"
#include "managers/display/display_manager.h"
#include "render/vk_types.h"
#include "vk_debug.h"
#include <vulkan-memory-allocator-hpp/vk_mem_alloc_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "vulkan-memory-allocator-hpp/vk_mem_alloc.hpp"

#include "VkBootstrap.h"
#include "debug/debug_draw.h"
#include "render/material_system.h"
#include "render/vk_descriptors.h"
#include "render/vk_initializers.h"
#include "render/vk_textures.h"
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_enums.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#define ASSET_PATH "resources/assets_export/"
#define SHADER_PATH "resources/shaders/"

RenderManager *RenderManager::get() {
	static RenderManager render_manager{};
	return &render_manager;
}

void RenderManager::init_vulkan(DisplayManager &display_manager) {
	vkb::InstanceBuilder builder;
	auto inst_ret = builder.set_app_name("Silence Vulkan Application")
							.request_validation_layers()
							.use_default_debug_messenger()
							.require_api_version(1, 1, 0)
							.enable_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
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

	vk::PhysicalDeviceFeatures features = {};
	features.multiDrawIndirect = VK_TRUE;
	features.fillModeNonSolid = VK_TRUE;

	selector.set_required_features(features);

	auto physical_device_ret = selector.set_minimum_version(1, 1)
									   .set_surface(surface)
									   //    .add_required_extension(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME)
									   .select();

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

	VkDebug::setup_debugging(this);

	gpu_properties = vkb_device.physical_device.properties;

	main_deletion_queue.push_function([=, this]() { allocator.destroy(); });
}

uint32_t previous_pow2(uint32_t v) {
	uint32_t r = 1;

	while (r * 2 < v) {
		r *= 2;
	}

	return r;
}
uint32_t get_image_mip_levels(uint32_t width, uint32_t height) {
	uint32_t result = 1;

	while (width > 1 || height > 1) {
		result++;
		width /= 2;
		height /= 2;
	}

	return result;
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

	//for the render, depth and pyramid images, we want to allocate them from gpu local memory
	vma::AllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = vma::MemoryUsage::eGpuOnly;
	dimg_allocinfo.requiredFlags = vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eDeviceLocal);

	// render image
	{
		//render image size will match the window
		vk::Extent3D render_image_extent = { window_extent.width, window_extent.height, 1 };
		render_format = vk::Format::eR32G32B32A32Sfloat;
		vk::ImageCreateInfo ri_info = vk_init::image_create_info(render_format,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc |
						vk::ImageUsageFlagBits::eSampled,
				render_image_extent);

		//allocate and create the image
		VK_CHECK(allocator.createImage(
				&ri_info, &dimg_allocinfo, &raw_render_image.image, &raw_render_image.allocation, nullptr));

		//build a image-view for the depth image to use for rendering
		vk::ImageViewCreateInfo dview_info =
				vk_init::image_view_create_info(render_format, raw_render_image.image, vk::ImageAspectFlagBits::eColor);

		VK_CHECK(device.createImageView(&dview_info, nullptr, &raw_render_image.default_view));

		main_deletion_queue.push_function([=, this]() {
			device.destroyImageView(raw_render_image.default_view);
			allocator.destroyImage(raw_render_image.image, raw_render_image.allocation);
		});
	}

	// depth image
	{
		//depth image size will match the window
		vk::Extent3D depth_image_extent = { window_extent.width, window_extent.height, 1 };

		//hardcoding the depth format to 32-bit float
		depth_format = vk::Format::eD32Sfloat;

		//the depth image will be an image with the format we selected and Depth Attachment usage flag
		vk::ImageCreateInfo dimg_info = vk_init::image_create_info(depth_format,
				vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, depth_image_extent);

		//allocate and create the image
		VK_CHECK(allocator.createImage(
				&dimg_info, &dimg_allocinfo, &depth_image.image, &depth_image.allocation, nullptr));

		//build an image-view for the depth image to use for render
		vk::ImageViewCreateInfo dview_info =
				vk_init::image_view_create_info(depth_format, depth_image.image, vk::ImageAspectFlagBits::eDepth);

		VK_CHECK(device.createImageView(&dview_info, nullptr, &depth_image.default_view));

		//add to deletion queues
		main_deletion_queue.push_function([=, this]() {
			device.destroyImageView(depth_image.default_view);
			allocator.destroyImage(depth_image.image, depth_image.allocation);
		});
	}

	// Note: previous_pow2 makes sure all reductions are at most by 2x2 which makes sure they are conservative
	depth_pyramid_width = (int)previous_pow2(window_extent.width);
	depth_pyramid_height = (int)previous_pow2(window_extent.height);
	depth_pyramid_levels = (int)get_image_mip_levels(depth_pyramid_width, depth_pyramid_height);

	vk::Extent3D pyramid_extent = { static_cast<uint32_t>(depth_pyramid_width),
		static_cast<uint32_t>(depth_pyramid_height), 1 };
	//the depth image will be a image with the format we selected and Depth Attachment usage flag
	vk::ImageCreateInfo pyramid_info = vk_init::image_create_info(vk::Format::eR32Sfloat,
			vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc,
			pyramid_extent);

	pyramid_info.mipLevels = depth_pyramid_levels;

	//allocate and create the image
	VK_CHECK(allocator.createImage(
			&pyramid_info, &dimg_allocinfo, &depth_pyramid.image, &depth_pyramid.allocation, nullptr));

	//build a image-view for the depth image to use for rendering
	vk::ImageViewCreateInfo preview_info = vk_init::image_view_create_info(
			vk::Format::eR32Sfloat, depth_pyramid.image, vk::ImageAspectFlagBits::eColor);
	preview_info.subresourceRange.levelCount = depth_pyramid_levels;

	VK_CHECK(device.createImageView(&preview_info, nullptr, &depth_pyramid.default_view));

	for (int32_t i = 0; i < depth_pyramid_levels; ++i) {
		vk::ImageViewCreateInfo level_info = vk_init::image_view_create_info(
				vk::Format::eR32Sfloat, depth_pyramid.image, vk::ImageAspectFlagBits::eColor);
		level_info.subresourceRange.levelCount = 1;
		level_info.subresourceRange.baseMipLevel = i;

		vk::ImageView pyramid;
		VK_CHECK(device.createImageView(&level_info, nullptr, &pyramid));

		depth_pyramid_mips[i] = pyramid;
		assert(depth_pyramid_mips[i]);

		main_deletion_queue.push_function([=, this]() { device.destroyImageView(pyramid); });
	}

	main_deletion_queue.push_function([=, this]() {
		device.destroyImageView(depth_pyramid.default_view);
		allocator.destroyImage(depth_pyramid.image, depth_pyramid.allocation);
	});

	vk::SamplerCreateInfo create_info = {};

	auto reduction_mode = vk::SamplerReductionMode::eMin;

	create_info.sType = vk::StructureType::eSamplerCreateInfo;
	create_info.magFilter = vk::Filter::eLinear;
	create_info.minFilter = vk::Filter::eLinear;
	create_info.mipmapMode = vk::SamplerMipmapMode::eNearest;
	create_info.addressModeU = vk::SamplerAddressMode::eClampToEdge;
	create_info.addressModeV = vk::SamplerAddressMode::eClampToEdge;
	create_info.addressModeW = vk::SamplerAddressMode::eClampToEdge;
	create_info.minLod = 0;
	create_info.maxLod = 16.f;

	// TODO: check if this is supported
	// vk::SamplerReductionModeCreateInfoEXT create_info_reduction = {};
	// create_info_reduction.sType = vk::StructureType::eSamplerReductionModeCreateInfoEXT;

	// if (reduction_mode != vk::SamplerReductionMode::eWeightedAverage) {
	// 	create_info_reduction.reductionMode = reduction_mode;

	// 	create_info.pNext = &create_info_reduction;
	// }

	VK_CHECK(device.createSampler(&create_info, nullptr, &depth_sampler));
	VkDebug::set_name(depth_sampler, "depth_sampler (init_swapchain)");

	vk::SamplerCreateInfo sampler_info = vk_init::sampler_create_info(vk::Filter::eLinear);
	sampler_info.mipmapMode = vk::SamplerMipmapMode::eLinear;

	VK_CHECK(device.createSampler(&sampler_info, nullptr, &smooth_sampler));
	VkDebug::set_name(depth_sampler, "smooth_sampler (init_swapchain)");

	main_deletion_queue.push_function([=, this]() {
		device.destroySampler(depth_sampler);
		device.destroySampler(smooth_sampler);
	});
}

void RenderManager::init_commands() {
	//create a command pool for commands submitted to the graphics queue.
	vk::CommandPoolCreateInfo command_pool_info = vk_init::command_pool_create_info(
			graphics_queue_family, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	for (auto &frame : frames) {
		frame.command_pool = device.createCommandPool(command_pool_info);

		//allocate the default command buffer that we will use for render
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

void RenderManager::init_forward_renderpass() {
	// the renderpass will use this color attachment.
	vk::AttachmentDescription color_attachment = {};
	//the attachment will have the HDR format
	color_attachment.format = vk::Format::eR32G32B32A32Sfloat;
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
	color_attachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

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

void RenderManager::init_copy_renderpass() {
	vk::AttachmentDescription color_attachment = {};
	color_attachment.format = swapchain_image_format;
	color_attachment.samples = vk::SampleCountFlagBits::e1;
	color_attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
	color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
	color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	color_attachment.initialLayout = vk::ImageLayout::eUndefined;
	color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

	//we are going to create 1 subpass, which is the minimum you can do
	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	//1 dependency, which is from "outside" into the subpass. And we can read or write color
	vk::SubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependency.srcAccessMask = vk::AccessFlagBits::eNone;
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

	vk::RenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = vk::StructureType::eRenderPassCreateInfo;
	//2 attachments from said array
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	//render_pass_info.dependencyCount = 1;
	//render_pass_info.pDependencies = &dependency;

	VK_CHECK(device.createRenderPass(&render_pass_info, nullptr, &copy_pass));

	main_deletion_queue.push_function([=]() { device.destroyRenderPass(copy_pass, nullptr); });
}

void RenderManager::init_framebuffers() {
	vk::FramebufferCreateInfo fwd_info = vk_init::framebuffer_create_info(render_pass, window_extent);
	vk::ImageView attachments[2];
	attachments[0] = raw_render_image.default_view;
	attachments[1] = depth_image.default_view;

	fwd_info.pAttachments = attachments;
	fwd_info.attachmentCount = 2;
	VK_CHECK(device.createFramebuffer(&fwd_info, nullptr, &forward_framebuffer));
	VkDebug::set_name(forward_framebuffer, "Forward Framebuffer");

	main_deletion_queue.push_function([=]() { device.destroyFramebuffer(forward_framebuffer, nullptr); });
	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for
	//render

	//grab how many images we have in the swapchain
	const uint32_t swapchain_imagecount = swapchain_images.size();
	framebuffers = std::vector<vk::Framebuffer>(swapchain_imagecount);

	//create framebuffers for each of the swapchain image views
	for (int i = 0; i < swapchain_imagecount; i++) {
		// We're reusing the same depth image view for all framebuffers, since we're only drawing one frame at a
		// time
		vk::FramebufferCreateInfo fb_info = vk_init::framebuffer_create_info(copy_pass, window_extent);
		fb_info.pAttachments = &swapchain_image_views[i];
		fb_info.attachmentCount = 1;
		VK_CHECK(device.createFramebuffer(&fb_info, nullptr, &framebuffers[i]));

		main_deletion_queue.push_function([=, this]() {
			device.destroyFramebuffer(framebuffers[i]);
			device.destroyImageView(swapchain_image_views[i]);
		});
	}

	//main_deletion_queue.push_function([=, this]() { device.destroyFramebuffer(forward_framebuffer); });
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
	descriptor_allocator = new vk_util::DescriptorAllocator();
	descriptor_allocator->init(device);

	descriptor_layout_cache = new vk_util::DescriptorLayoutCache();
	descriptor_layout_cache->init(device);

	const size_t scene_param_buffer_size = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));

	scene_parameter_buffer = create_buffer("Scene parameter buffer", scene_param_buffer_size,
			vk::BufferUsageFlagBits::eUniformBuffer, vma::MemoryUsage::eCpuToGpu);

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

	global_set_layout = descriptor_layout_cache->create_descriptor_layout(&setinfo);

	vk::DescriptorSetLayoutBinding object_bind = vk_init::descriptor_set_layout_binding(
			vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex, 0);
	vk::DescriptorSetLayoutCreateInfo object_set_info = {};
	object_set_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
	object_set_info.pNext = nullptr;
	object_set_info.bindingCount = 1;
	object_set_info.flags = {};
	object_set_info.pBindings = &object_bind;

	object_set_layout = descriptor_layout_cache->create_descriptor_layout(&object_set_info);

	vk::DescriptorSetLayoutBinding texture1_bind = vk_init::descriptor_set_layout_binding(
			vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 0);

	vk::DescriptorSetLayoutBinding texture2_bind = vk_init::descriptor_set_layout_binding(
			vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1);

	vk::DescriptorSetLayoutBinding texture_binds[] = { texture1_bind, texture2_bind };

	vk::DescriptorSetLayoutCreateInfo set3_info = {};
	set3_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
	set3_info.pNext = nullptr;
	set3_info.bindingCount = 2;
	set3_info.flags = {};
	set3_info.pBindings = &texture_binds[0];

	single_texture_set_layout = descriptor_layout_cache->create_descriptor_layout(&set3_info);

	for (auto &frame : frames) {
		frame.dynamic_descriptor_allocator = new vk_util::DescriptorAllocator();
		frame.dynamic_descriptor_allocator->init(device);

		frame.camera_buffer = create_buffer("Camera buffer", sizeof(GPUCameraData),
				vk::BufferUsageFlagBits::eUniformBuffer, vma::MemoryUsage::eCpuToGpu);

		// TODO: Move this constant somewhere else
		const int max_objects = 10000;
		frame.object_buffer = create_buffer("Object buffer", max_objects * sizeof(GPUObjectData),
				vk::BufferUsageFlagBits::eStorageBuffer, vma::MemoryUsage::eCpuToGpu);

		// TODO: Move this constant somewhere else
		const int max_commands = 10000;

		frame.indirect_buffer = create_buffer("Indirect buffer", max_commands * sizeof(vk::DrawIndexedIndirectCommand),
				vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eTransferDst |
						vk::BufferUsageFlagBits::eStorageBuffer,
				vma::MemoryUsage::eCpuToGpu);

		// 1 megabyte of dynamic data buffer
		frame.dynamic_buffer = create_buffer(
				"Dynamic data buffer", 1000000, vk::BufferUsageFlagBits::eUniformBuffer, vma::MemoryUsage::eCpuOnly);
		frame.dynamic_data.init(allocator, frame.dynamic_buffer, gpu_properties.limits.minUniformBufferOffsetAlignment);
	}

	main_deletion_queue.push_function([&]() {
		allocator.destroyBuffer(scene_parameter_buffer.buffer, scene_parameter_buffer.allocation);

		for (auto &frame : frames) {
			allocator.destroyBuffer(frame.camera_buffer.buffer, frame.camera_buffer.allocation);
			allocator.destroyBuffer(frame.object_buffer.buffer, frame.object_buffer.allocation);
			allocator.destroyBuffer(frame.indirect_buffer.buffer, frame.indirect_buffer.allocation);
			frame.dynamic_data.free(allocator);
			allocator.destroyBuffer(frame.dynamic_buffer.buffer, frame.dynamic_buffer.allocation);
		}
	});
}

void RenderManager::init_pipelines() {
	material_system.init(this);
	material_system.build_default_templates();

	//fullscreen triangle pipeline for blits
	auto *blit_effect = new ShaderEffect();
	blit_effect->add_stage(
			shader_cache.get_shader(shader_path("fullscreen.vert.spv")), vk::ShaderStageFlagBits::eVertex);
	blit_effect->add_stage(shader_cache.get_shader(shader_path("blit.frag.spv")), vk::ShaderStageFlagBits::eFragment);
	blit_effect->reflect_layout(device, nullptr, 0);

	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader
	//modules per stage
	PipelineBuilder pipeline_builder;

	//vertex input controls how to read vertices from vertex buffers. We aren't using it yet
	pipeline_builder.vertex_input_info = vk_init::vertex_input_state_create_info();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw_objects triangle list
	pipeline_builder.input_assembly = vk_init::input_assembly_create_info(vk::PrimitiveTopology::eTriangleList);

	//build viewport and scissor from the swapchain extents
	//
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
	pipeline_builder.rasterizer.cullMode = vk::CullModeFlagBits::eNone;

	//we don't use multisampling, so just run the default one
	pipeline_builder.multisampling = vk_init::multisampling_state_create_info();

	//a single blend attachment with no blending and writing to RGBA
	pipeline_builder.color_blend_attachment = vk_init::color_blend_attachment_state();

	//default depthtesting
	pipeline_builder.depth_stencil = vk_init::depth_stencil_create_info(true, true, vk::CompareOp::eGreaterOrEqual);

	//build the blit pipeline
	pipeline_builder.set_shaders(blit_effect);

	//blit pipeline uses hardcoded triangle so no need for vertex input
	pipeline_builder.clear_vertex_input();

	pipeline_builder.depth_stencil = vk_init::depth_stencil_create_info(false, false, vk::CompareOp::eAlways);

	blit_pipeline = pipeline_builder.build_pipeline(device, copy_pass);
	blit_pipeline_layout = blit_effect->built_layout;

	VkDebug::set_name(blit_pipeline, "Blit pipeline");
	VkDebug::set_name(blit_pipeline_layout, "Blit pipeline layout");

	main_deletion_queue.push_function([&]() { device.destroyPipeline(blit_pipeline); });

	//load the compute shaders
	load_compute_shader(shader_path("indirect_cull.comp.spv").c_str(), cull_pipeline, cull_layout);
	load_compute_shader(shader_path("depth_reduce.comp.spv").c_str(), depth_reduce_pipeline, depth_reduce_layout);
	load_compute_shader(shader_path("sparse_upload.comp.spv").c_str(), sparse_upload_pipeline, sparse_upload_layout);

	main_deletion_queue.push_function([=, this]() {
		device.destroyPipeline(mesh_pipeline);
		device.destroyPipelineLayout(mesh_pipeline_layout);
	});
}

bool RenderManager::load_compute_shader(const char *shader_path, vk::Pipeline &pipeline, vk::PipelineLayout &layout) {
	ShaderModule compute_module;
	if (!vk_util::load_shader_module(device, shader_path, &compute_module))

	{
		std::cout << "Error when building compute shader shader module" << std::endl;
		return false;
	}

	auto *compute_effect = new ShaderEffect();

	compute_effect->add_stage(&compute_module, vk::ShaderStageFlagBits::eCompute);

	compute_effect->reflect_layout(device, nullptr, 0);

	ComputePipelineBuilder compute_builder;
	compute_builder.pipeline_layout = compute_effect->built_layout;
	compute_builder.shader_stage =
			vk_init::pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eCompute, compute_module.module);

	layout = compute_effect->built_layout;
	pipeline = compute_builder.build_pipeline(device);

	main_deletion_queue.push_function([=]() { device.destroyPipeline(pipeline, nullptr); });

	return true;
}

void RenderManager::init_debug_vertex_buffer() {
	const uint32_t max_debug_verts = 100000;

	// Create vertex buffer from vertices
	size_t buffer_size = max_debug_verts * sizeof(DebugVertex);
	//allocate staging buffer
	vk::BufferCreateInfo buffer_info = {};
	buffer_info.sType = vk::StructureType::eBufferCreateInfo;
	buffer_info.pNext = nullptr;

	buffer_info.size = buffer_size;
	buffer_info.usage = vk::BufferUsageFlagBits::eVertexBuffer;

	//let the VMA library know that this data should be on CPU RAM
	vma::AllocationCreateInfo vmaalloc_info = {};
	vmaalloc_info.usage = vma::MemoryUsage::eCpuToGpu;

	//allocate the buffer
	VK_CHECK(allocator.createBuffer(
			&buffer_info, &vmaalloc_info, &debug_vertex_buffer.buffer, &debug_vertex_buffer.allocation, nullptr));

	VkDebug::set_name(debug_vertex_buffer.buffer, "Debug draw vertex buffer");

	main_deletion_queue.push_function(
			[=]() { allocator.destroyBuffer(debug_vertex_buffer.buffer, debug_vertex_buffer.allocation); });
}

void RenderManager::init_scene() {
	// //create a sampler for the texture
	// vk::SamplerCreateInfo sampler_info = vk_init::sampler_create_info(vk::Filter::eNearest);

	// vk::Sampler blocky_sampler;
	// VK_CHECK(device.createSampler(&sampler_info, nullptr, &blocky_sampler));

	// sampler_info.mipmapMode = vk::SamplerMipmapMode::eLinear;
	// // TODO: check this
	// // sampler_info.mipLodBias = 2;
	// sampler_info.maxLod = 30;
	// sampler_info.minLod = 3;

	// vk::Sampler smooth_sampler2;
	// VK_CHECK(device.createSampler(&sampler_info, nullptr, &smooth_sampler2));

	// {
	// 	vk_util::MaterialData textured_info = {};
	// 	textured_info.base_template = "textured";
	// 	textured_info.parameters = nullptr;

	// 	vk_util::SampledTexture white_tex = {};
	// 	white_tex.sampler = smooth_sampler2;
	// 	white_tex.image_view = loaded_textures["white"].image_view;

	// 	textured_info.textures.push_back(white_tex);

	// 	vk_util::Material *new_mat = material_system.build_material("textured", textured_info);
	// }

	// load_prefab(asset_path("DamagedHelmet/DamagedHelmet.pfb").c_str());
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
}

void RenderManager::load_meshes() {
	// triangle_mesh.vertices.resize(3);

	// //vertex positions
	// triangle_mesh.vertices[0].position = { 1.f, 1.f, 0.0f };
	// triangle_mesh.vertices[1].position = { -1.f, 1.f, 0.0f };
	// triangle_mesh.vertices[2].position = { 0.f, -1.f, 0.0f };

	// //vertex colors, all green
	// triangle_mesh.vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
	// triangle_mesh.vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	// triangle_mesh.vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green

	// triangle_mesh.indices = { 0, 1, 2 };

	// //we don't care about the vertex normals

	// upload_mesh(triangle_mesh);

	// meshes["triangle"] = triangle_mesh;
}

void RenderManager::upload_mesh(Mesh &mesh) {
	size_t buffer_size = mesh.vertices.size() * sizeof(Vertex);
	//allocate staging buffer
	vk::BufferCreateInfo buffer_info = {};
	buffer_info.sType = vk::StructureType::eBufferCreateInfo;
	buffer_info.pNext = nullptr;

	buffer_info.size = buffer_size;
	buffer_info.usage = vk::BufferUsageFlagBits::eTransferSrc;

	//let the VMA library know that this data should be on CPU RAM
	vma::AllocationCreateInfo vmaalloc_info = {};
	vmaalloc_info.usage = vma::MemoryUsage::eCpuOnly;

	//allocate the buffer
	VK_CHECK(allocator.createBuffer(
			&buffer_info, &vmaalloc_info, &mesh.vertex_buffer.buffer, &mesh.vertex_buffer.allocation, nullptr));

	void *data;
	VK_CHECK(allocator.mapMemory(mesh.vertex_buffer.allocation, &data));
	memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
	allocator.unmapMemory(mesh.vertex_buffer.allocation);

	//add the destruction of triangle mesh buffer to the deletion queue
	main_deletion_queue.push_function(
			[=, this]() { allocator.destroyBuffer(mesh.vertex_buffer.buffer, mesh.vertex_buffer.allocation); });

	// Create new buffer, this time for indices
	buffer_size = mesh.indices.size() * sizeof(uint32_t);
	buffer_info.size = buffer_size;

	VK_CHECK(allocator.createBuffer(
			&buffer_info, &vmaalloc_info, &mesh.index_buffer.buffer, &mesh.index_buffer.allocation, nullptr));

	VK_CHECK(allocator.mapMemory(mesh.index_buffer.allocation, &data));
	memcpy(data, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));
	allocator.unmapMemory(mesh.index_buffer.allocation);

	//add the destruction of triangle mesh buffer to the deletion queue
	main_deletion_queue.push_function(
			[=, this]() { allocator.destroyBuffer(mesh.index_buffer.buffer, mesh.index_buffer.allocation); });
}

bool RenderManager::load_image_to_cache(const char *name, const char *path) {
	Texture newtex;

	if (loaded_textures.find(name) != loaded_textures.end()) {
		return true;
	}

	bool result = vk_util::load_image_from_asset(*this, path, newtex.image);

	if (!result) {
		SPDLOG_ERROR("Error When texture {} at path {}", name, path);
		return false;
	} else {
		SPDLOG_INFO("Loaded texture {} at path {}", name, path);
	}
	newtex.image_view = newtex.image.default_view;
	//VkImageViewCreateInfo imageinfo = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_UNORM, newtex.image._image,
	//VK_IMAGE_ASPECT_COLOR_BIT); imageinfo.subresourceRange.levelCount = newtex.image.mipLevels;
	//vkCreateImageView(_device, &imageinfo, nullptr, &newtex.imageView);

	loaded_textures[name] = newtex;
	return true;
}

PrefabInstance RenderManager::load_prefab(const char *path, const glm::mat4 &root) {
	auto pf = prefab_cache.find(path);
	if (pf == prefab_cache.end()) {
		assets::AssetFile file;
		bool loaded = assets::load_binary_file(path, file);

		if (!loaded) {
			SPDLOG_ERROR("Error When loading prefab file at path {}", path);
			assert(false);
		} else {
			SPDLOG_INFO("Prefab {} loaded to cache", path);
		}

		prefab_cache[path] = new assets::PrefabInfo;

		*prefab_cache[path] = assets::read_prefab_info(&file);
	}

	assets::PrefabInfo *prefab = prefab_cache[path];

	vk::SamplerCreateInfo sampler_info = vk_init::sampler_create_info(vk::Filter::eLinear);
	sampler_info.mipmapMode = vk::SamplerMipmapMode::eLinear;

	std::unordered_map<uint64_t, glm::mat4> node_worldmats;

	std::vector<std::pair<uint64_t, glm::mat4>> pending_nodes;
	for (auto &[k, v] : prefab->node_matrices) {
		glm::mat4 nodematrix{ 1.f };

		auto nm = prefab->matrices[v];
		memcpy(&nodematrix, &nm, sizeof(glm::mat4));

		//check if it has parents
		auto matrix_it = prefab->node_parents.find(k);
		if (matrix_it == prefab->node_parents.end()) {
			//add to worldmats
			node_worldmats[k] = root * nodematrix;
		} else {
			//enqueue
			pending_nodes.emplace_back(k, nodematrix);
		}
	}

	//process pending nodes list until it empties
	while (!pending_nodes.empty()) {
		for (int i = 0; i < pending_nodes.size(); i++) {
			uint64_t node = pending_nodes[i].first;
			uint64_t parent = prefab->node_parents[node];

			//try to find parent in cache
			auto matrix_it = node_worldmats.find(parent);
			if (matrix_it != node_worldmats.end()) {
				//transform with the parent
				glm::mat4 nodematrix = (matrix_it)->second * pending_nodes[i].second;

				node_worldmats[node] = nodematrix;

				//remove from queue, pop last
				pending_nodes[i] = pending_nodes.back();
				pending_nodes.pop_back();
				i--;
			}
		}
	}

	std::vector<MeshObject> prefab_renderables;
	prefab_renderables.reserve(prefab->node_meshes.size());

	for (auto &[k, v] : prefab->node_meshes) {
		//load mesh

		if (v.mesh_path.find("Sky") != std::string::npos) {
			continue;
		}

		if (!get_mesh(v.mesh_path)) {
			Mesh mesh{};
			mesh.load_from_asset(asset_path(v.mesh_path).c_str());

			upload_mesh(mesh);

			meshes[v.mesh_path.c_str()] = mesh;
		}

		auto material_name = v.material_path.c_str();
		//load material

		vk_util::Material *object_material = material_system.get_material(material_name);
		if (!object_material) {
			assets::AssetFile material_file;
			bool loaded = assets::load_binary_file(asset_path(material_name).c_str(), material_file);

			if (loaded) {
				assets::MaterialInfo material = assets::read_material_info(&material_file);
				std::vector<std::string> textures;
				std::string black_texture_path = "resources/assets_export/black.ktx2";

				// BASE COLOR
				auto base_color_tex = material.textures["baseColor"];
				loaded = load_image_to_cache(base_color_tex.c_str(), asset_path(base_color_tex).c_str());
				if (!loaded) {
					loaded = load_image_to_cache(base_color_tex.c_str(), black_texture_path.c_str());
					SPDLOG_WARN("Loaded black picture as base color texture {}", base_color_tex);
				}
				textures.push_back(base_color_tex);

				// AMBIENT OCCLUSION
				auto ao_tex = material.textures["occlusion"];
				loaded = load_image_to_cache(ao_tex.c_str(), asset_path(ao_tex).c_str());
				if (!loaded) {
					loaded = load_image_to_cache(ao_tex.c_str(), black_texture_path.c_str());
					SPDLOG_WARN("Loaded black picture as ambient occlusion texture {}", ao_tex);
				}
				textures.push_back(ao_tex);

				// NORMAL
				auto normal_tex = material.textures["normals"];
				loaded = load_image_to_cache(normal_tex.c_str(), asset_path(normal_tex).c_str());
				if (!loaded) {
					loaded = load_image_to_cache(normal_tex.c_str(), black_texture_path.c_str());
					SPDLOG_WARN("Loaded black picture as normal texture {}", normal_tex);
				}
				textures.push_back(normal_tex);

				// METALLIC ROUGHNESS
				auto metallic_roughness_tex = material.textures["metallicRoughness"];
				loaded =
						load_image_to_cache(metallic_roughness_tex.c_str(), asset_path(metallic_roughness_tex).c_str());
				if (!loaded) {
					loaded = load_image_to_cache(metallic_roughness_tex.c_str(), black_texture_path.c_str());
					SPDLOG_WARN("Loaded black picture as metallicRoughness texture {}", metallic_roughness_tex);
				}
				textures.push_back(metallic_roughness_tex);

				// EMISSIVE
				auto emissive_tex = material.textures["emissive"];
				loaded = load_image_to_cache(emissive_tex.c_str(), asset_path(emissive_tex).c_str());
				if (!loaded) {
					loaded = load_image_to_cache(emissive_tex.c_str(), black_texture_path.c_str());
					SPDLOG_WARN("Loaded black picture as emissive texture {}", emissive_tex);
				}
				textures.push_back(emissive_tex);

				vk_util::MaterialData info;
				info.parameters = nullptr;

				if (material.transparency == assets::TransparencyMode::Transparent) {
					info.base_template = "texturedPBR_transparent";
				} else {
					info.base_template = "texturedPBR_opaque";
				}

				for (auto &texture : textures) {
					// TODO: Check if we can reuse one sampler for multiple textures
					vk::Sampler smooth_sampler;
					VK_CHECK(device.createSampler(&sampler_info, nullptr, &smooth_sampler));
					VkDebug::set_name(smooth_sampler, fmt::format("Smooth Sampler for {}", texture).c_str());

					main_deletion_queue.push_function([=]() { device.destroySampler(smooth_sampler); });

					vk_util::SampledTexture tex;
					tex.image_view = loaded_textures[texture].image_view;
					tex.sampler = smooth_sampler;

					info.textures.push_back(tex);
				}

				object_material = material_system.build_material(material_name, info);

				if (!object_material) {
					SPDLOG_ERROR("Error When building material {}", v.material_path);
				}
			} else {
				SPDLOG_ERROR("Error When loading material at path {}", v.material_path);
			}
		}

		MeshObject load_mesh;
		//transparent objects will be invisible

		load_mesh.b_draw_forward_pass = true;
		load_mesh.b_draw_shadow_pass = true;

		glm::mat4 nodematrix{ 1.f };

		auto matrix_it = node_worldmats.find(k);
		if (matrix_it != node_worldmats.end()) {
			auto nm = (*matrix_it).second;
			memcpy(&nodematrix, &nm, sizeof(glm::mat4));
		}

		load_mesh.mesh = get_mesh(v.mesh_path);
		load_mesh.transform_matrix = nodematrix;
		load_mesh.material = object_material;

		// TODO: implement this
		// refresh_renderbounds(&load_mesh);

		//sort key from location
		auto lx = int(load_mesh.bounds.origin.x / 10.f);
		auto ly = int(load_mesh.bounds.origin.y / 10.f);

		uint32_t key = uint32_t(std::hash<int32_t>()(lx) ^ std::hash<int32_t>()(ly ^ 1337));

		load_mesh.custom_sort_key = 0; // rng;// key;

		prefab_renderables.push_back(load_mesh);
		//_renderables.push_back(load_mesh);
	}

	auto handles = render_scene.register_object_batch(
			prefab_renderables.data(), static_cast<uint32_t>(prefab_renderables.size()));

	needs_to_rebuild_objects = true;

	return PrefabInstance{ .object_ids = handles };
}

AllocatedBufferUntyped RenderManager::create_buffer(const std::string &allocation_name, size_t alloc_size,
		vk::BufferUsageFlags usage, vma::MemoryUsage memory_usage) const {
	//allocate vertex buffer
	vk::BufferCreateInfo buffer_info = {};
	buffer_info.sType = vk::StructureType::eBufferCreateInfo;
	buffer_info.pNext = nullptr;

	buffer_info.size = alloc_size;
	buffer_info.usage = usage;

	vma::AllocationCreateInfo vmaalloc_info = {};
	vmaalloc_info.usage = memory_usage;
	vmaalloc_info.pUserData = nullptr;

	AllocatedBufferUntyped new_buffer;

	//allocate the buffer
	VK_CHECK(allocator.createBuffer(&buffer_info, &vmaalloc_info, &new_buffer.buffer, &new_buffer.allocation, nullptr));

	vmaSetAllocationName(allocator, new_buffer.allocation, allocation_name.c_str());
	new_buffer.size = alloc_size;

	return new_buffer;
}

void RenderManager::reallocate_buffer(AllocatedBufferUntyped &buffer, size_t alloc_size, vk::BufferUsageFlags usage,
		vma::MemoryUsage memory_usage, vk::MemoryPropertyFlags required_flags) {
	// TODO: Add required_flags to create_buffer()
	AllocatedBufferUntyped new_buffer = create_buffer("Reallocation of buffer", alloc_size, usage, memory_usage);

	get_current_frame().frame_deletion_queue.push_function(
			[=, this]() { allocator.destroyBuffer(buffer.buffer, buffer.allocation); });

	main_deletion_queue.push_function(
			[=, this]() { allocator.destroyBuffer(new_buffer.buffer, new_buffer.allocation); });

	buffer = new_buffer;
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

void RenderManager::unmap_buffer(AllocatedBufferUntyped &buffer) const {
	allocator.unmapMemory(buffer.allocation);
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

	cmd.end();

	vk::SubmitInfo submit = vk_init::submit_info(&cmd);

	//submit command buffer to the queue and execute it.
	// _uploadFence will now block until the graphic commands finish execution
	VK_CHECK(graphics_queue.submit(1, &submit, upload_context.upload_fence));

	VK_CHECK(device.waitForFences(1, &upload_context.upload_fence, true, 9999999999));
	VK_CHECK(device.resetFences(1, &upload_context.upload_fence));

	// reset the command buffers inside the command pool
	device.resetCommandPool(upload_context.command_pool);
}

std::string RenderManager::asset_path(std::string_view path) {
	return std::string(ASSET_PATH) + std::string(path);
}

std::string RenderManager::shader_path(std::string_view path) {
	return std::string(SHADER_PATH) + std::string(path);
}

void RenderManager::ready_cull_data(RenderScene::MeshPass &pass, vk::CommandBuffer cmd) {
	if (pass.batches.empty()) {
		return;
	}

	//copy from the cleared indirect buffer into the one we will use on rendering. This one happens every frame
	vk::BufferCopy indirect_copy;
	indirect_copy.dstOffset = 0;
	indirect_copy.size = pass.batches.size() * sizeof(GPUIndirectObject);
	indirect_copy.srcOffset = 0;
	cmd.copyBuffer(pass.clear_indirect_buffer.buffer, pass.draw_indirect_buffer.buffer, 1, &indirect_copy);

	{
		vk::BufferMemoryBarrier barrier =
				vk_init::buffer_barrier(pass.draw_indirect_buffer.buffer, graphics_queue_family);
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;

		cull_ready_barriers.push_back(barrier);
		//vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr,
		//1, &barrier, 0, nullptr);
	}
}

RenderManager::Status RenderManager::startup(DisplayManager &display_manager, Camera *cam) {
	camera = cam;

	// TODO: Handle failures
	init_vulkan(display_manager);

	shader_cache.init(device);
	render_scene.init();

	init_swapchain(display_manager);

	init_forward_renderpass();
	init_copy_renderpass();

	init_framebuffers();
	init_commands();
	init_sync_structures();
	init_descriptors();
	init_pipelines();

	init_debug_vertex_buffer();

	load_images();
	load_meshes();

	init_scene();

	init_imgui(display_manager.window);

	vk::CommandBuffer cmd = get_current_frame().main_command_buffer;
	TracyVkContext(chosen_gpu, device, graphics_queue, cmd);

	return Status::Ok;
}

void RenderManager::shutdown() {
	//make sure the GPU has stopped doing its things
	device.waitIdle();
	main_deletion_queue.flush();

	for (auto &frame : frames) {
		frame.dynamic_descriptor_allocator->cleanup();
	}

	descriptor_allocator->cleanup();
	descriptor_layout_cache->cleanup();

	device.destroy();
	instance.destroySurfaceKHR(surface);
	vkb::destroy_debug_utils_messenger(instance, debug_messenger);
	instance.destroy();
}

FrameData &RenderManager::get_current_frame() {
	return frames[frame_number % FRAME_OVERLAP];
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
	load_image_to_cache("white", asset_path("missing.ktx2").c_str());
}

void RenderManager::draw(Camera &camera) {
	ZoneNamedC(Zone1, 0xff0000, true);
	if (needs_to_rebuild_objects) {
		render_scene.build_batches();
		render_scene.merge_meshes(this);
		needs_to_rebuild_objects = false;
	}

	//wait until the GPU has finished render the last frame. Timeout of 1 second
	VK_CHECK(device.waitForFences(1, &get_current_frame().render_fence, true, 1000000000));
	VK_CHECK(device.resetFences(1, &get_current_frame().render_fence));

	get_current_frame().frame_deletion_queue.flush();
	get_current_frame().dynamic_descriptor_allocator->reset_pools();

	// render_scene.build_batches();

	//request image from the swapchain, one second timeout
	uint32_t swapchain_image_index;
	VK_CHECK(device.acquireNextImageKHR(
			swapchain, 1000000000, get_current_frame().present_semaphore, nullptr, &swapchain_image_index));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin
	//recording again.
	get_current_frame().main_command_buffer.reset();

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

	post_cull_barriers.clear();
	cull_ready_barriers.clear();

	ready_mesh_draw(cmd);

	ready_cull_data(render_scene.forward_pass, cmd);

	cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, {}, 0, nullptr,
			cull_ready_barriers.size(), cull_ready_barriers.data(), 0, nullptr);

	glm::vec3 cam_pos = { 0.f, 3.f, -10.f };

	glm::mat4 view = glm::translate(glm::mat4(1.f), cam_pos);
	//camera projection
	float aspect = (float)window_extent.width / (float)window_extent.height;
	glm::mat4 projection = glm::perspective(glm::radians(70.f), aspect, 0.1f, 200.0f);

	CullParams forward_cull{};
	forward_cull.projmat = projection;
	forward_cull.viewmat = view;
	forward_cull.frustrum_cull = false;
	forward_cull.occlusion_cull = false;
	forward_cull.draw_dist = 3000;
	forward_cull.aabb = false;
	execute_compute_cull(cmd, render_scene.forward_pass, forward_cull);

	cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eDrawIndirect, {}, 0,
			nullptr, post_cull_barriers.size(), post_cull_barriers.data(), 0, nullptr);

	forward_pass(cmd);

	reduce_depth(cmd);

	copy_render_to_swapchain(cmd, swapchain_image_index);

	cmd.end();

	vk::SubmitInfo submit = vk_init::submit_info(&cmd);
	vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	submit.pWaitDstStageMask = &wait_stage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &get_current_frame().present_semaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &get_current_frame().render_semaphore;
	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(graphics_queue.submit(1, &submit, get_current_frame().render_fence));
	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that,
	// as its necessary that drawing commands have finished before the image is displayed to the user
	vk::PresentInfoKHR present_info = vk_init::present_info();

	present_info.pSwapchains = &swapchain;
	present_info.swapchainCount = 1;

	present_info.pWaitSemaphores = &get_current_frame().render_semaphore;
	present_info.waitSemaphoreCount = 1;

	present_info.pImageIndices = &swapchain_image_index;

	VK_CHECK(graphics_queue.presentKHR(&present_info));

	/*

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

	draw_objects(camera, cmd);

	{
		ZoneNamedNC(Zone3, "ImGui", 0x980dd4, true);
		ImGui::Render();

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	}

	cmd.endRenderPass();
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	cmd.end();

	//prepare the submission to the queue.
	//we want to wait on the _present_semaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the render_semaphore, to signal that render has finished

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
*/

	frame_number++;
}

void RenderManager::forward_pass(vk::CommandBuffer cmd) {
	//clear depth at 0
	vk::ClearValue depth_clear;
	depth_clear.depthStencil.depth = 0.f;

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	vk::RenderPassBeginInfo rp_info = vk_init::renderpass_begin_info(
			render_pass, window_extent, forward_framebuffer /*framebuffers[swapchainImageIndex]*/);

	//connect clear values
	rp_info.clearValueCount = 2;

	vk::ClearValue clear_value{};
	clear_value.color.setFloat32({ 0.01f, 0.01f, 0.01f, 1.0f });

	vk::ClearValue clear_values[] = { clear_value, depth_clear };

	rp_info.pClearValues = &clear_values[0];
	cmd.beginRenderPass(&rp_info, vk::SubpassContents::eInline);

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = (float)window_extent.height;
	viewport.width = (float)window_extent.width;
	viewport.height = -(float)window_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor;
	scissor.setOffset({ 0, 0 });
	scissor.extent = window_extent;

	cmd.setViewport(0, 1, &viewport);
	cmd.setScissor(0, 1, &scissor);
	cmd.setDepthBias(0, 0, 0);

	draw_objects_forward(cmd, render_scene.forward_pass);

	ImGui::Render();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	//finalize the render pass
	cmd.endRenderPass();
}

void RenderManager::copy_render_to_swapchain(vk::CommandBuffer cmd, uint32_t swapchain_image_index) {
	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	vk::RenderPassBeginInfo copy_rp =
			vk_init::renderpass_begin_info(copy_pass, window_extent, framebuffers[swapchain_image_index]);

	cmd.beginRenderPass(&copy_rp, vk::SubpassContents::eInline);

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)window_extent.width;
	viewport.height = (float)window_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor;
	scissor.setOffset({ 0, 0 });
	scissor.extent = window_extent;

	cmd.setViewport(0, 1, &viewport);
	cmd.setScissor(0, 1, &scissor);

	cmd.setDepthBias(0, 0, 0);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, blit_pipeline);

	vk::DescriptorImageInfo source_image;
	source_image.sampler = smooth_sampler;

	source_image.imageView = raw_render_image.default_view;
	source_image.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::DescriptorSet blit_set;
	vk_util::DescriptorBuilder::begin(descriptor_layout_cache, get_current_frame().dynamic_descriptor_allocator)
			.bind_image(0, &source_image, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
			.build(blit_set);

	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, blit_pipeline_layout, 0, 1, &blit_set, 0, nullptr);

	cmd.draw(3, 1, 0, 0);

	cmd.endRenderPass();
}