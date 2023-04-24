#include "vk_mesh.h"

#include "assets/mesh_asset.h"

VertexInputDescription Vertex::get_vertex_description() {
	VertexInputDescription description;

	//we will have just 1 vertex buffer binding, with a per-vertex rate
	vk::VertexInputBindingDescription main_binding = {};
	main_binding.binding = 0;
	main_binding.stride = sizeof(Vertex);
	main_binding.inputRate = vk::VertexInputRate::eVertex;

	description.bindings.push_back(main_binding);

	//Position will be stored at Location 0
	vk::VertexInputAttributeDescription position_attribute = {};
	position_attribute.binding = 0;
	position_attribute.location = 0;
	position_attribute.format = vk::Format::eR32G32B32A32Sfloat;
	position_attribute.offset = offsetof(Vertex, position);

	//Normal will be stored at Location 1
	vk::VertexInputAttributeDescription normal_attribute = {};
	normal_attribute.binding = 0;
	normal_attribute.location = 1;
	normal_attribute.format = vk::Format::eR32G32B32A32Sfloat;
	normal_attribute.offset = offsetof(Vertex, normal);

	//Color will be stored at Location 2
	vk::VertexInputAttributeDescription color_attribute = {};
	color_attribute.binding = 0;
	color_attribute.location = 2;
	color_attribute.format = vk::Format::eR32G32B32A32Sfloat;
	color_attribute.offset = offsetof(Vertex, color);

	//UV will be stored at Location 3
	vk::VertexInputAttributeDescription uv_attribute = {};
	uv_attribute.binding = 0;
	uv_attribute.location = 3;
	uv_attribute.format = vk::Format::eR32G32Sfloat;
	uv_attribute.offset = offsetof(Vertex, uv);

	description.attributes.push_back(position_attribute);
	description.attributes.push_back(normal_attribute);
	description.attributes.push_back(color_attribute);
	description.attributes.push_back(uv_attribute);
	return description;
}

bool Mesh::load_from_asset(const char *filename) {
	assets::AssetFile file;
	bool loaded = assets::load_binary_file(filename, file);

	if (!loaded) {
		SPDLOG_ERROR("Couldn't load mesh asset {}", filename);
		return false;
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

		vertices[i].color[0] = unpacked_vertices[i].color[0];
		vertices[i].color[1] = unpacked_vertices[i].color[1];
		vertices[i].color[2] = unpacked_vertices[i].color[2];

		vertices[i].uv[0] = unpacked_vertices[i].uv[0];
		vertices[i].uv[1] = unpacked_vertices[i].uv[1];
	}

	// Load indices
	auto *unpacked_indices = (uint32_t *)loaded_indices.data();
	indices.resize(loaded_indices.size() / sizeof(uint32_t));

	for (size_t i = 0; i < indices.size(); i++) {
		indices[i] = unpacked_indices[i];
	}

	SPDLOG_INFO("Mesh asset loaded successfully: {}", filename);

	return true;
}
