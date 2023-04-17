#ifndef SILENCE_VK_TYPES_H
#define SILENCE_VK_TYPES_H

#if 1
#define VMA_DEBUG_LOG(format, ...)                                                                                     \
	do {                                                                                                               \
		printf(format, ##__VA_ARGS__);                                                                                 \
		printf("\n");                                                                                                  \
	} while (false)
#endif

#include "vulkan-memory-allocator-hpp/vk_mem_alloc.hpp"

#define VK_CHECK(x)                                                                                                    \
	do {                                                                                                               \
		vk::Result err = x;                                                                                            \
		if (err != vk::Result::eSuccess) {                                                                             \
			SPDLOG_ERROR("Detected Vulkan error: ({}) {}", magic_enum::enum_integer(err), magic_enum::enum_name(err)); \
			abort();                                                                                                   \
		}                                                                                                              \
	} while (false)

struct AllocatedBufferUntyped {
	vk::Buffer buffer{};
	vma::Allocation allocation{};
	vk::DeviceSize size{ 0 };
	vk::DescriptorBufferInfo get_info(vk::DeviceSize offset = 0);
};

template <typename T> struct AllocatedBuffer : public AllocatedBufferUntyped {
	AllocatedBuffer<T> &operator=(const AllocatedBufferUntyped &other) {
		buffer = other.buffer;
		allocation = other.allocation;
		size = other.size;
		return *this;
	}
	AllocatedBuffer(AllocatedBufferUntyped &other) {
		buffer = other.buffer;
		allocation = other.allocation;
		size = other.size;
	}
	AllocatedBuffer(AllocatedBufferUntyped other) {
		buffer = other.buffer;
		allocation = other.allocation;
		size = other.size;
	}
	AllocatedBuffer() = default;
};

inline vk::DescriptorBufferInfo AllocatedBufferUntyped::get_info(vk::DeviceSize offset) {
	vk::DescriptorBufferInfo info;
	info.buffer = buffer;
	info.offset = offset;
	info.range = size;
	return info;
}

struct AllocatedImage {
	vk::Image image;
	vma::Allocation allocation;
	vk::ImageView default_view;
};

enum class MeshpassType : uint8_t { None = 0, Forward = 1, Transparency = 2, DirectionalShadow = 3 };

#endif //SILENCE_VK_TYPES_H
