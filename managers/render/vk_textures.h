#ifndef SILENCE_VK_TEXTURES_H
#define SILENCE_VK_TEXTURES_H

#include "render_manager.h"
#include "vk_types.h"

namespace vk_util {

bool load_image_from_asset(RenderManager &manager, const char *filename, AllocatedImage &out_image);

AllocatedImage upload_image(RenderManager &manager, uint32_t tex_width, uint32_t tex_height, vk::Format image_format,
		AllocatedBufferUntyped &staging_buffer);

}; //namespace vk_util

#endif //SILENCE_VK_TEXTURES_H
