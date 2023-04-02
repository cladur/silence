#ifndef SILENCE_VK_TEXTURES_H
#define SILENCE_VK_TEXTURES_H

#include "render_manager.h"
#include "vk_types.h"

namespace vk_util {

bool load_image_from_file(RenderManager &manager, const char *file, AllocatedImage &out_image);

};

#endif //SILENCE_VK_TEXTURES_H
