#ifndef SILENCE_SPRITE_MANAGER_H
#define SILENCE_SPRITE_MANAGER_H

#include <render/common/shader.h>
#include <render/common/texture.h>

class SpriteManager {
private:
	std::unordered_map<std::string, Texture> sprite_textures;

public:
	static SpriteManager *get();
	void startup();
	void shutdown();
	void load_sprite_texture(const char *name, const char *path);
	Texture &get_sprite_texture(const std::string &);
};

#endif //SILENCE_SPRITE_MANAGER_H
