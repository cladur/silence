#include "texture_atlas.h"

extern RenderManager render_manager;

TextureAtlas::TextureAtlas(glm::ivec2 size) : size(size) {
	current_x = 0;
	texture = render_manager.create_sized_texture(size.x, size.y);
	render_manager.transition_image_layout(
			texture.image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferDstOptimal);

	render_manager.transition_image_layout(
			texture.image,
			vk::ImageLayout::eTransferDstOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal);

	sampler = render_manager.create_sampler(vk::Filter::eLinear, vk::SamplerAddressMode::eClampToBorder);

	render_manager.create_descriptor_set_for_material(texture.image_view, sampler, "ui");
	render_manager.update_descriptor_set_with_texture(texture, sampler, *render_manager.get_material("ui"));
}

TextureAtlas::~TextureAtlas() = default;

glm::ivec2 TextureAtlas::get_size() {
	return size;
}

int TextureAtlas::add(void *pixels, glm::ivec2 size) {
	vk::DeviceSize buffer_size = size.x * size.y * 4;
	AllocatedBuffer staging_buffer = render_manager.create_buffer(
			buffer_size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vma::MemoryUsage::eCpuCopy);
	render_manager.copy_to_buffer(staging_buffer, pixels, buffer_size);
	render_manager.transition_image_layout(
			texture.image,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageLayout::eTransferDstOptimal);
	render_manager.copy_buffer_to_image(staging_buffer, texture.image, size.x, size.y, current_x, 0);
	render_manager.transition_image_layout(
			texture.image,
			vk::ImageLayout::eTransferDstOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal);

	render_manager.destroy_buffer(staging_buffer);

	current_x += size.x;
	return current_x;
}
