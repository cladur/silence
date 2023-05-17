#include "skinned_mesh.h"

#include "glad/glad.h"

#include "assets/skinned_mesh_asset.h"

void SkinnedMesh::draw() {
	// draw mesh
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void SkinnedMesh::setup_mesh() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SkinnedMeshVertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedMeshVertex), (void *)nullptr);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
			1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedMeshVertex), (void *)offsetof(SkinnedMeshVertex, normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedMeshVertex), (void *)offsetof(SkinnedMeshVertex, uv));
	// vertex weights
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
			3, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedMeshVertex), (void *)offsetof(SkinnedMeshVertex, weight));
	// vertex bone_ids
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 4, GL_INT, sizeof(SkinnedMeshVertex), (void *)offsetof(SkinnedMeshVertex, joint_id));

	glBindVertexArray(0);
}

void SkinnedMesh::load_from_asset(const char *path) {
	assets::AssetFile file;
	bool loaded = assets::load_binary_file(path, file);

	if (!loaded) {
		SPDLOG_ERROR("Couldn't load skinned mesh asset {}", path);
	}

	assets::SkinnedMeshInfo mesh_info = assets::read_skinned_mesh_info(&file);

	std::vector<char> loaded_vertices;
	std::vector<char> loaded_indices;

	loaded_vertices.resize(mesh_info.vertex_buffer_size);
	loaded_indices.resize(mesh_info.index_buffer_size);

	assets::unpack_mesh(&mesh_info, file.binary_blob.data(), file.binary_blob.size(), loaded_vertices.data(),
			loaded_indices.data());

	vertices.clear();
	indices.clear();

	// Load vertices
	auto *unpacked_vertices = (assets::SkinnedVertexPNV32 *)loaded_vertices.data();
	vertices.resize(loaded_vertices.size() / sizeof(assets::SkinnedVertexPNV32));

	for (size_t i = 0; i < vertices.size(); i++) {
		vertices[i].position[0] = unpacked_vertices[i].position[0];
		vertices[i].position[1] = unpacked_vertices[i].position[1];
		vertices[i].position[2] = unpacked_vertices[i].position[2];

		vertices[i].normal[0] = unpacked_vertices[i].normal[0];
		vertices[i].normal[1] = unpacked_vertices[i].normal[1];
		vertices[i].normal[2] = unpacked_vertices[i].normal[2];

		vertices[i].uv[0] = unpacked_vertices[i].uv[0];
		vertices[i].uv[1] = unpacked_vertices[i].uv[1];

		vertices[i].weight[0] = unpacked_vertices[i].weight[0];
		vertices[i].weight[1] = unpacked_vertices[i].weight[1];
		vertices[i].weight[2] = unpacked_vertices[i].weight[2];
		vertices[i].weight[3] = unpacked_vertices[i].weight[3];

		vertices[i].joint_id[0] = unpacked_vertices[i].joint[0];
		vertices[i].joint_id[1] = unpacked_vertices[i].joint[1];
		vertices[i].joint_id[2] = unpacked_vertices[i].joint[2];
		vertices[i].joint_id[3] = unpacked_vertices[i].joint[3];
	}

	// Load indices
	auto *unpacked_indices = (uint32_t *)loaded_indices.data();
	indices.resize(loaded_indices.size() / sizeof(uint32_t));

	for (size_t i = 0; i < indices.size(); i++) {
		indices[i] = unpacked_indices[i];
	}
	//	static int index = 0;
	//	for (auto &s : vertices) {
	//		SPDLOG_INFO("vertex {}", index++);
	//		SPDLOG_INFO("w {} {} {} {}", s.weight[0], s.weight[1], s.weight[2], s.weight[3]);
	//		SPDLOG_INFO("i {} {} {} {}", s.joint_id[0], s.joint_id[1], s.joint_id[2], s.joint_id[3]);
	//	}

	setup_mesh();

	SPDLOG_INFO("Skinned Mesh asset loaded successfully: {}", path);
}
