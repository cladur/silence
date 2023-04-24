#ifndef SILENCE_MESH_H
#define SILENCE_MESH_H

#include "texture.h"

struct Shader;

struct MeshVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

class Mesh {
public:
	// mesh data
	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture> textures;

	void draw(Shader &shader);
	void setup_mesh();
	void load_from_asset(const char *path);

	//  render data
	unsigned int vao, vbo, ebo;
};

#endif // SILENCE_MESH_H