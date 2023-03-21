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
