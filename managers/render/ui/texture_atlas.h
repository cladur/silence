#ifndef SILENCE_TEXTURE_ATLAS_H
#define SILENCE_TEXTURE_ATLAS_H

#include <render/render_manager.h>

// FOR NOW LETS ASSUME IT WILL BE JUST A LONG 1 x N TEXTURE for simplicity
class TextureAtlas {
private:
	Texture texture;
	glm::ivec2 size;
	vk::Sampler sampler;
	int current_x = 0;

public:
	TextureAtlas(glm::ivec2 size);
	~TextureAtlas();
	int add(void *pixels, glm::ivec2 size);
	glm::ivec2 get_size();
};

#endif //SILENCE_TEXTURE_ATLAS_H
