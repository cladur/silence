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
	// PBR: [0] = albedo, [1] = normal, [2] = ao | metallic | roughness, [3] = emissive
	std::array<Texture, 4> textures;
	std::array<bool, 4> textures_present;

	bool has_ao_map = false;

	void draw();
	void setup_mesh();
	void load_from_asset(const char *path);
	
	std::vector<glm::vec3> get_position_vertices() const;

	//  render data
	unsigned int vao, vbo, ebo;
};

#endif // SILENCE_MESH_H