#include "vk_push_buffer.h"

uint32_t vk_util::PushBuffer::push(void *data, size_t size) {
	uint32_t offset = current_offset;
	char *target = (char *)mapped;
	target += current_offset;
	memcpy(target, data, size);
	current_offset += static_cast<uint32_t>(size);
	current_offset = pad_uniform_buffer_size(current_offset);

	return offset;
}

void vk_util::PushBuffer::init(vma::Allocator &allocator, AllocatedBufferUntyped source_buffer, uint32_t alignement) {
	align = alignement;
	source = source_buffer;
	current_offset = 0;
	VK_CHECK(allocator.mapMemory(source_buffer.allocation, &mapped));
}

void vk_util::PushBuffer::reset() {
	current_offset = 0;
}

uint32_t vk_util::PushBuffer::pad_uniform_buffer_size(uint32_t original_size) {
	// Calculate required alignment based on minimum device offset alignment
	size_t min_ubo_alignment = align;
	size_t aligned_size = original_size;
	if (min_ubo_alignment > 0) {
		aligned_size = (aligned_size + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);
	}
	return static_cast<uint32_t>(aligned_size);
}