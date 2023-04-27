#ifndef SILENCE_TEXTURE_H
#define SILENCE_TEXTURE_H

#include "glad/glad.h"

struct Texture {
	uint32_t id;
	uint32_t width;
	uint32_t height;
	uint32_t channels;

	void load_from_asset(const char *path);
};

#endif // SILENCE_TEXTURE_H