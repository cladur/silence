#include "vk_textures.h"

#include "spdlog/spdlog.h"

#include "vk_initializers.h"

// #define STB_IMAGE_IMPLEMENTATION
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>

bool vk_util::load_image_from_file(RenderManager &engine, const char *file, AllocatedImage &out_image) {
	int tex_width, tex_height, tex_channels;

	stbi_uc *pixels = stbi_load(file, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

	if (!pixels) {
		SPDLOG_ERROR("Failed to load image from file: {}", file);
		return false;
	}

	void *pixel_ptr = pixels;
	vk::DeviceSize image_size = tex_width * tex_height * 4;

	//the format R8G8B8A8 matches exactly with the pixels loaded from stb_image lib
	vk::Format image_format = vk::Format::eR8G8B8A8Srgb;

	//allocate temporary buffer for holding texture data to upload
	AllocatedBuffer staging_buffer =
			engine.create_buffer(image_size, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuOnly);

	//copy data to buffer
	void *data;
	VK_CHECK(engine.allocator.mapMemory(staging_buffer.allocation, &data));

	memcpy(data, pixel_ptr, static_cast<size_t>(image_size));

	engine.allocator.unmapMemory(staging_buffer.allocation);
	//we no longer need the loaded data, so we can free the pixels as they are now in the staging buffer
	stbi_image_free(pixels);

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
	VK_CHECK(engine.allocator.createImage(
			&dimg_info, &dimg_allocinfo, &new_image.image, &new_image.allocation, nullptr));

	engine.immediate_submit([&](vk::CommandBuffer cmd) {
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

	engine.main_deletion_queue.push_function(
			[=]() { engine.allocator.destroyImage(new_image.image, new_image.allocation); });

	engine.allocator.destroyBuffer(staging_buffer.buffer, staging_buffer.allocation);

	SPDLOG_INFO("Texture loaded successfully: {}", file);

	out_image = new_image;

	return true;
}
