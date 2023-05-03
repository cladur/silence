#include "sprite_manager.h"
SpriteManager *SpriteManager::get() {
	static SpriteManager sprite_manager;
	return &sprite_manager;
}

void SpriteManager::startup() {
	load_sprite_texture("default", "missing.ktx2");
}

void SpriteManager::shutdown() {
}

void SpriteManager::load_sprite_texture(const char *name, const char *path) {
	if (sprite_textures.find(path) != sprite_textures.end()) {
		return;
	}
	Texture texture = {};
	texture.load_from_asset(asset_path(path).c_str());
	sprite_textures[name] = texture;
}

Texture &SpriteManager::get_sprite_texture(const std::string& name) {
	// if not found return empty texture
	if (sprite_textures.find(name) == sprite_textures.end()) {
		return sprite_textures["default"];
	}
	return sprite_textures[name.c_str()];
}
