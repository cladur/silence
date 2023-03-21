#include "render_manager.h"
#include "display_manager.h"

#include "rendering/vk_initializers.h"
#include <VkBootstrap.h>
#include <spdlog/spdlog.h>

#define VK_CHECK(x)                                         \
	do                                                      \
	{                                                       \
		VkResult err = x;                                   \
		if (err)                                            \
		{                                                   \
			SPDLOG_ERROR("Detected Vulkan error: {}", err); \
			abort();                                        \
		}                                                   \
	} while (false)

void RenderManager::init_vulkan(DisplayManager &display_manager)
{
	vkb::InstanceBuilder builder;
	auto inst_ret = builder.set_app_name("Silence Vulkan Application")
							.request_validation_layers()
							.use_default_debug_messenger()
							.build();
	if (!inst_ret)
	{
		SPDLOG_ERROR("Failed to create Vulkan Instance. Error: {}", inst_ret.error().message());
	}

	auto vkb_inst = inst_ret.value();

	//store the instance
	instance = vkb_inst.instance;
	//store the debug messenger
	debug_messenger = vkb_inst.debug_messenger;

	surface = display_manager.create_surface(vkb_inst.instance);

	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	vkb::PhysicalDevice physical_device = selector
												  .set_minimum_version(1, 1)
												  .set_surface(surface)
												  .select()
												  .value();

	//create the final Vulkan device
	vkb::DeviceBuilder device_builder{ physical_device };

	vkb::Device vkb_device = device_builder.build().value();

	// Get the VkDevice handle used in the rest of a Vulkan application
	device = vkb_device.device;
	chosen_gpu = physical_device.physical_device;

	// use vkbootstrap to get a Graphics queue
	graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
	graphics_queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
}

void RenderManager::init_swapchain(DisplayManager &display_manager)
{
	auto window_size = display_manager.get_window_size();

	vkb::SwapchainBuilder swapchain_builder{ chosen_gpu, device, surface };

	auto vkbSwapchain = swapchain_builder
								.use_default_format_selection()
								// We use VSync present mode for now
								// TODO: make this configurable if we want to uncap FPS in future
								.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
								.set_desired_extent(window_size.first, window_size.second)
								.build()
								.value();

	//store swapchain and its related images
	swapchain = vkbSwapchain.swapchain;
	auto temp_swapchain_images = vkbSwapchain.get_images().value();
	swapchain_images = std::vector<vk::Image>(temp_swapchain_images.begin(), temp_swapchain_images.end());
	auto temp_swapchain_image_views = vkbSwapchain.get_image_views().value();
	swapchain_image_views = std::vector<vk::ImageView>(temp_swapchain_image_views.begin(), temp_swapchain_image_views.end());

	swapchain_image_format = static_cast<vk::Format>(vkbSwapchain.image_format);
}

void RenderManager::init_commands()
{
	//create a command pool for commands submitted to the graphics queue.
	vk::CommandPoolCreateInfo command_pool_info = vk_init::command_pool_create_info(
			graphics_queue_family,
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	auto command_pool_res = device.createCommandPool(command_pool_info);

	if (command_pool_res.result != vk::Result::eSuccess)
	{
		SPDLOG_ERROR("Failed to create command pool");
		assert(false);
	}

	command_pool = command_pool_res.value;

	//allocate the default command buffer that we will use for rendering
	vk::CommandBufferAllocateInfo cmdAllocInfo = vk_init::command_buffer_allocate_info(command_pool, 1);

	auto main_command_buffer_res = device.allocateCommandBuffers(cmdAllocInfo);

	if (main_command_buffer_res.result != vk::Result::eSuccess)
	{
		SPDLOG_ERROR("Failed to allocate command buffer");
		assert(false);
	}

	main_command_buffer = main_command_buffer_res.value[0];
}

RenderManager::Status RenderManager::startup(DisplayManager &display_manager)
{
	// TODO: Handle failures
	init_vulkan(display_manager);
	init_swapchain(display_manager);
	init_commands();

	return Status::Ok;
}

void RenderManager::shutdown()
{
	device.destroyCommandPool(command_pool);

	// Destroy swapchain using Vulkan-hpp
	device.destroySwapchainKHR(swapchain);

	//destroy swapchain resources
	for (auto &swapchain_image_view : swapchain_image_views)
	{
		device.destroyImageView(swapchain_image_view);
	}

	device.destroy();
	instance.destroySurfaceKHR(surface);
	vkb::destroy_debug_utils_messenger(instance, debug_messenger);
	instance.destroy();
}
