#ifndef SILENCE_TEXTURE_ASSET_H
#define SILENCE_TEXTURE_ASSET_H

#include "asset_loader.h"

namespace assets {
enum class TextureFormat : uint32_t { Unknown = 0, RGBA8 };

struct TextureInfo {
	uint64_t texture_size;
	TextureFormat texture_format;
	CompressionMode compression_mode;
	uint32_t pixel_size[3];
	std::string original_file;
};

//parses the texture metadata from an asset file
TextureInfo read_texture_info(AssetFile *file);

AssetFile pack_texture(TextureInfo *info, void *pixel_data);
void unpack_texture(TextureInfo *info, const char *source_buffer, size_t source_size, char *destination);
} //namespace assets

#endif //SILENCE_TEXTURE_ASSET_H
