#ifndef SILENCE_MODEL_H
#define SILENCE_MODEL_H

#include "mesh.h"

class Model {
public:
	Model(const char *path) {
		load_from_asset(path);
	}

	void draw(Shader &shader);

	bool is_refractive = false;
	glm::mat4 transform_matrix{};

	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;

private:
	// model data
	std::string directory;

	void load_from_asset(std::string path);
};

#endif // SILENCE_MODEL_H