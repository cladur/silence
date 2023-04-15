#include "vk_textures.h"

#include "assets/texture_asset.h"

#include "render/vk_types.h"
#include "vk_initializers.h"

// #define STB_IMAGE_IMPLEMENTATION
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>

bool vk_util::load_image_from_asset(RenderManager &manager, const char *filename, AllocatedImage &out_image) {
	assets::AssetFile file;
	bool loaded = assets::load_binary_file(filename, file);

	if (!loaded) {
		SPDLOG_ERROR("Couldn't load image asset {}", filename);
		return false;
	}

	assets::TextureInfo texture_info = assets::read_texture_info(&file);

	vk::DeviceSize image_size = texture_info.texture_size;
	vk::Format image_format;
	switch (texture_info.texture_format) {
		case assets::TextureFormat::RGBA8:
			image_format = vk::Format::eR8G8B8A8Unorm;
			break;
		default:
			SPDLOG_ERROR("Encountered unsupported texture format while loading image asset {}", filename);
			return false;
	}

	//allocate temporary buffer for holding texture data to upload
	AllocatedBufferUntyped staging_buffer = manager.create_buffer(
			"Staging buffer", image_size, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuOnly);

	//copy data to buffer
	void *data;
	VK_CHECK(manager.allocator.mapMemory(staging_buffer.allocation, &data));

	//	memcpy(data, pixel_ptr, static_cast<size_t>(image_size));
	assets::unpack_texture(&texture_info, file.binary_blob.data(), file.binary_blob.size(), (char *)data);

	manager.allocator.unmapMemory(staging_buffer.allocation);

	out_image =
			upload_image(manager, texture_info.pixel_size[0], texture_info.pixel_size[1], image_format, staging_buffer);

	manager.allocator.destroyBuffer(staging_buffer.buffer, staging_buffer.allocation);

	SPDLOG_INFO("Texture asset loaded successfully: {}", filename);

	return true;
}

AllocatedImage vk_util::upload_image(RenderManager &manager, uint32_t tex_width, uint32_t tex_height,
		vk::Format image_format, AllocatedBufferUntyped &staging_buffer) {
	vk::Extent3D image_extent;
	image_extent.width = static_cast<uint32_t>(tex_width);
	image_extent.height = static_cast<uint32_t>(tex_height);
	image_extent.depth = 1;

	vk::ImageCreateInfo dimg_info = vk_init::image_create_info(
			image_format, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, image_extent);

	AllocatedImage new_image;

	vma::AllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = vma::MemoryUsage::eGpuOnly;

	//allocate and create the image
	VK_CHECK(manager.allocator.createImage(
			&dimg_info, &dimg_allocinfo, &new_image.image, &new_image.allocation, nullptr));

	manager.immediate_submit([&](vk::CommandBuffer cmd) {
		vk::ImageSubresourceRange range;
		range.aspectMask = vk::ImageAspectFlagBits::eColor;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		vk::ImageMemoryBarrier image_barrier_to_transfer = {};
		image_barrier_to_transfer.sType = vk::StructureType::eImageMemoryBarrier;

		image_barrier_to_transfer.oldLayout = vk::ImageLayout::eUndefined;
		image_barrier_to_transfer.newLayout = vk::ImageLayout::eTransferDstOptimal;
		image_barrier_to_transfer.image = new_image.image;
		image_barrier_to_transfer.subresourceRange = range;

		image_barrier_to_transfer.srcAccessMask = {};
		image_barrier_to_transfer.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		//barrier the image into the transfer-receive layout
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr,
				0, nullptr, 1, &image_barrier_to_transfer);

		vk::BufferImageCopy copy_region = {};
		copy_region.bufferOffset = 0;
		copy_region.bufferRowLength = 0;
		copy_region.bufferImageHeight = 0;

		copy_region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		copy_region.imageSubresource.mipLevel = 0;
		copy_region.imageSubresource.baseArrayLayer = 0;
		copy_region.imageSubresource.layerCount = 1;
		copy_region.imageExtent = image_extent;

		//copy the buffer into the image
		cmd.copyBufferToImage(
				staging_buffer.buffer, new_image.image, vk::ImageLayout::eTransferDstOptimal, 1, &copy_region);

		vk::ImageMemoryBarrier image_barrier_to_readable = image_barrier_to_transfer;

		image_barrier_to_readable.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		image_barrier_to_readable.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		image_barrier_to_readable.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		image_barrier_to_readable.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		//barrier the image into the shader readable layout
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0,
				nullptr, 0, nullptr, 1, &image_barrier_to_readable);
	});

	manager.main_deletion_queue.push_function(
			[=]() { manager.allocator.destroyImage(new_image.image, new_image.allocation); });

	return new_image;
}
