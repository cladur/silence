#ifndef SILENCE_VK_INITIALIZERS_H
#define SILENCE_VK_INITIALIZERS_H

#include "vulkan/vulkan.hpp"

namespace vk_init {

vk::CommandPoolCreateInfo command_pool_create_info(uint32_t queue_family_index, vk::CommandPoolCreateFlags flags);

vk::CommandBufferAllocateInfo command_buffer_allocate_info(vk::CommandPool pool, uint32_t count = 1, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
}; //namespace vk_init

#endif //SILENCE_VK_INITIALIZERS_H
