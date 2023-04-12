#include "vk_types.h"

namespace vk_util {
struct PushBuffer {
	template <typename T> uint32_t push(T &data);

	uint32_t push(void *data, size_t size);

	void init(vma::Allocator &allocator, AllocatedBufferUntyped source_buffer, uint32_t alignement);
	void reset();

	uint32_t pad_uniform_buffer_size(uint32_t original_size);
	AllocatedBufferUntyped source;
	uint32_t align;
	uint32_t current_offset;
	void *mapped;
};

template <typename T> uint32_t vk_util::PushBuffer::push(T &data) {
	return push(&data, sizeof(T));
}

} //namespace vk_util