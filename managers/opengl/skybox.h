#ifndef SILENCE_SKYBOX_H
#define SILENCE_SKYBOX_H

#include "texture.h"

class Skybox {
private:
	unsigned int vao, vbo;

public:
	Texture skybox_map;
	Texture irradiance_map;
	Texture prefilter_map;
	Texture brdf_lut;

	void draw() const;

	void startup();
	void load_from_directory(const std::string &path);
};

#endif //SILENCE_SKYBOX_H
