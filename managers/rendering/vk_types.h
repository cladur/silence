#ifndef SILENCE_VK_TYPES_H
#define SILENCE_VK_TYPES_H

#include "vulkan-memory-allocator-hpp/vk_mem_alloc.hpp"

struct AllocatedBuffer {
	vk::Buffer buffer;
	vma::Allocation allocation;
};

struct AllocatedImage {
	vk::Image image;
	vma::Allocation allocation;
};

#endif //SILENCE_VK_TYPES_H
