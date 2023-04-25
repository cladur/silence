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
	// PBR: [0] = albedo, [1] = ao, [2] = normal, [3] = metallic | roughness, [4] = emissive
	std::array<Texture, 5> textures;
	std::array<bool, 5> textures_present;

	void draw(Shader &shader);
	void setup_mesh();
	void load_from_asset(const char *path);

	//  render data
	unsigned int vao, vbo, ebo;
};

#endif // SILENCE_MESH_H