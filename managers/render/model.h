#ifndef SILENCE_MODEL_H
#define SILENCE_MODEL_H

#include "mesh.h"

class Model {
public:
	void load_from_asset(const char *path);

	glm::mat4 root{};

	std::vector<Mesh> meshes;

private:
	// model data
	std::string directory;
};

#endif // SILENCE_MODEL_H