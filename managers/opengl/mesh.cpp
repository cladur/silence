#include "mesh.h"

#include "glad/glad.h"

#include "assets/mesh_asset.h"

#include "shader.h"

int TextureTypeToTextureUnit(std::string type) {
	if (type == "albedo_map") {
		return 3;
	} else if (type == "normal_map") {
		return 4;
	} else if (type == "metallic_map") {
		return 5;
	} else if (type == "roughness_map") {
		return 6;
	} else if (type == "ao_map") {
		return 7;
	} else if (type == "emission_map") {
		return 8;
	}

	SPDLOG_ERROR("Unknown texture type: {}", type);
	assert(false);
	return 0;
}

void Mesh::draw(MaterialType material_type) {
	switch (material_type) {
		case MATERIAL_TYPE_UNLIT: {
			bind_textures_unlit();
			break;
		}
		case MATERIAL_TYPE_PBR: {
			bind_textures_pbr();
			break;
		}
		default: {
			SPDLOG_ERROR("Unknown material type: {}", material_type);
			assert(false);
		}
	}

	// draw mesh
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void Mesh::bind_textures_pbr() {
	for (int i = 0; i < textures.size(); i++) {
		if (textures_present[i]) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
	}
}

void Mesh::bind_textures_unlit() {
	// Just albedo
	if (textures_present[0]) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[0].id);
	}
}

void Mesh::setup_mesh() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MeshVertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void *)nullptr);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void *)offsetof(MeshVertex, normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void *)offsetof(MeshVertex, uv));

	glBindVertexArray(0);
}

void Mesh::load_from_asset(const char *path) {
	assets::AssetFile file;
	bool loaded = assets::load_binary_file(path, file);

	if (!loaded) {
		SPDLOG_ERROR("Couldn't load mesh asset {}", path);
	}

	assets::MeshInfo mesh_info = assets::read_mesh_info(&file);

	std::vector<char> loaded_vertices;
	std::vector<char> loaded_indices;

	loaded_vertices.resize(mesh_info.vertex_buffer_size);
	loaded_indices.resize(mesh_info.index_buffer_size);

	assets::unpack_mesh(&mesh_info, file.binary_blob.data(), file.binary_blob.size(), loaded_vertices.data(),
			loaded_indices.data());

	vertices.clear();
	indices.clear();

	// Load vertices
	auto *unpacked_vertices = (assets::VertexPNV32 *)loaded_vertices.data();
	vertices.resize(loaded_vertices.size() / sizeof(assets::VertexPNV32));

	for (size_t i = 0; i < vertices.size(); i++) {
		vertices[i].position[0] = unpacked_vertices[i].position[0];
		vertices[i].position[1] = unpacked_vertices[i].position[1];
		vertices[i].position[2] = unpacked_vertices[i].position[2];

		vertices[i].normal[0] = unpacked_vertices[i].normal[0];
		vertices[i].normal[1] = unpacked_vertices[i].normal[1];
		vertices[i].normal[2] = unpacked_vertices[i].normal[2];

		vertices[i].uv[0] = unpacked_vertices[i].uv[0];
		vertices[i].uv[1] = unpacked_vertices[i].uv[1];
	}

	// Load indices
	auto *unpacked_indices = (uint32_t *)loaded_indices.data();
	indices.resize(loaded_indices.size() / sizeof(uint32_t));

	for (size_t i = 0; i < indices.size(); i++) {
		indices[i] = unpacked_indices[i];
	}

	setup_mesh();

	SPDLOG_INFO("Mesh asset loaded successfully: {}", path);
}