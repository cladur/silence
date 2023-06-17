#ifndef SILENCE_TEXTURE_H
#define SILENCE_TEXTURE_H

#include "glad/glad.h"
#include <ktx.h>

struct Texture {
	uint32_t id;
	uint32_t width;
	uint32_t height;
	uint32_t channels;

	static std::map<std::string, ktxTexture2 *> ktx_textures;
	//static std::set<std::string> ktx_paths;

	void load_from_asset(const std::string &path, bool pregenerated_mipmaps = false, bool repeat = true);
};

#endif // SILENCE_TEXTURE_H