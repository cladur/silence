#include "vk_mesh.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

namespace tg = tinygltf;

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

bool Mesh::load_from_gltf(const char *filename) {
	tg::Model model;
	std::string err;
	std::string warn;

	static tg::TinyGLTF loader;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);

	if (!ret) {
		SPDLOG_ERROR("Failed to load gltf file: {} - {} - {}", filename, err, warn);
		return false;
	}

	if (!warn.empty()) {
		SPDLOG_WARN("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		SPDLOG_ERROR("Err: %s\n", err.c_str());
		return false;
	}

	// Load vertices and indices from the gltf
	for (const auto &mesh : model.meshes) {
		for (const auto &primitive : mesh.primitives) {
			const auto &attributes = primitive.attributes;

			// Load vertices
			const auto &position_accessor = model.accessors[attributes.find("POSITION")->second];
			const auto &position_buffer_view = model.bufferViews[position_accessor.bufferView];
			const auto &position_buffer = model.buffers[position_buffer_view.buffer];

			const auto &normal_accessor = model.accessors[attributes.find("NORMAL")->second];
			const auto &normal_buffer_view = model.bufferViews[normal_accessor.bufferView];
			const auto &normal_buffer = model.buffers[normal_buffer_view.buffer];

			const auto &uv_accessor = model.accessors[attributes.find("TEXCOORD_0")->second];
			const auto &uv_buffer_view = model.bufferViews[uv_accessor.bufferView];
			const auto &uv_buffer = model.buffers[uv_buffer_view.buffer];

			const auto &position_data = reinterpret_cast<const float *>(
					&position_buffer.data[position_buffer_view.byteOffset + position_accessor.byteOffset]);
			const auto &normal_data = reinterpret_cast<const float *>(
					&normal_buffer.data[normal_buffer_view.byteOffset + normal_accessor.byteOffset]);
			const auto &uv_data = reinterpret_cast<const float *>(
					&uv_buffer.data[uv_buffer_view.byteOffset + uv_accessor.byteOffset]);

			for (size_t i = 0; i < position_accessor.count; i++) {
				Vertex vertex = {};
				vertex.position = glm::vec3(position_data[i * 3], position_data[i * 3 + 1], position_data[i * 3 + 2]);
				vertex.normal = glm::vec3(normal_data[i * 3], normal_data[i * 3 + 1], normal_data[i * 3 + 2]);
				vertex.color = vertex.normal;
				vertex.uv = glm::vec2(uv_data[i * 2], uv_data[i * 2 + 1]);
				vertices.push_back(vertex);
			}

			// Load indices
			const auto &index_accessor = model.accessors[primitive.indices];
			const auto &index_buffer_view = model.bufferViews[index_accessor.bufferView];
			const auto &gltf_index_buffer = model.buffers[index_buffer_view.buffer];

			const auto &index_data = &gltf_index_buffer.data[index_buffer_view.byteOffset + index_accessor.byteOffset];

			for (size_t i = 0; i < index_accessor.count; i++) {
				switch (index_accessor.componentType) {
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
						indices.push_back(reinterpret_cast<const uint32_t *>(index_data)[i]);
						break;
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
						indices.push_back(reinterpret_cast<const uint16_t *>(index_data)[i]);
						break;
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
						indices.push_back(reinterpret_cast<const uint8_t *>(index_data)[i]);
						break;
					default:
						break;
				}
			}
		}
	}

	return true;
}
