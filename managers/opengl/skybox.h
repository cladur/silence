#ifndef SILENCE_SKYBOX_H
#define SILENCE_SKYBOX_H

#include "texture.h"

class Skybox {
private:
	unsigned int vao, vbo;

public:
	Texture skybox_map;
	Texture irradiance_map;

	void draw() const;

	void startup();
	void load_skybox_map(const std::string &path);
	void load_irradiance_map(const std::string &path);
};

#endif //SILENCE_SKYBOX_H
