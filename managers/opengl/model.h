#ifndef SILENCE_MODEL_H
#define SILENCE_MODEL_H

#include "mesh.h"

class Model {
public:
	void draw(Shader &shader);
	void load_from_asset(const char *path);

	bool is_refractive = false;
	glm::mat4 root{};

	std::vector<Mesh> meshes;

private:
	// model data
	std::string directory;
};

#endif // SILENCE_MODEL_H