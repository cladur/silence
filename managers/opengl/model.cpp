#include "model.h"

#include <stb_image.h>

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);

void Model::draw(Shader &shader) {
	for (Mesh &mesh : meshes)
		mesh.draw(shader);
}

void Model::load_from_asset(std::string path) {
}
