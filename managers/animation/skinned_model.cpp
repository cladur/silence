#include "skinned_model.h"

bool SkinnedModel::load_model(const char *filename) {
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool result = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
	if (!result) {
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

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (int32_t node : scene.nodes) {
		assert((node >= 0) && (node < model.nodes.size()));
		// load skinned vertices
		process_node(model.nodes[node], node);
	}

	if (model.skins.empty()) {
		SPDLOG_WARN("Skinned model has no skin {}", filename);
	} else {
		// load joints
		process_skin(model.skins[0]);
	}

	return result;
}

void SkinnedModel::process_node(const tinygltf::Node &node, int32_t node_id) {
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		process_mesh(model.meshes[node.mesh]);
	}

	for (int32_t child : node.children) {
		assert((child >= 0) && (child < model.nodes.size()));
		process_node(model.nodes[child], child);
	}
}

void SkinnedModel::process_mesh(const tinygltf::Mesh &mesh) {
	std::vector<SkinnedVertex> vertices;
	std::vector<uint32_t> indices;

	for (const tinygltf::Primitive &primitive : mesh.primitives) {
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

		const auto &joint_accessor = model.accessors[attributes.find("JOINTS_0")->second];
		const auto &joint_buffer_view = model.bufferViews[joint_accessor.bufferView];
		const auto &joint_buffer = model.buffers[joint_buffer_view.buffer];

		const auto &weight_accessor = model.accessors[attributes.find("WEIGHTS_0")->second];
		const auto &weight_buffer_view = model.bufferViews[weight_accessor.bufferView];
		const auto &weight_buffer = model.buffers[weight_buffer_view.buffer];

		const auto &position_data = reinterpret_cast<const float *>(
				&position_buffer.data[position_buffer_view.byteOffset + position_accessor.byteOffset]);

		const auto &normal_data = reinterpret_cast<const float *>(
				&normal_buffer.data[normal_buffer_view.byteOffset + normal_accessor.byteOffset]);

		const auto &uv_data =
				reinterpret_cast<const float *>(&uv_buffer.data[uv_buffer_view.byteOffset + uv_accessor.byteOffset]);

		const auto &bone_data = reinterpret_cast<const int16_t *>(
				&joint_buffer.data[joint_buffer_view.byteOffset + joint_accessor.byteOffset]);

		const auto &weight_data = reinterpret_cast<const float *>(
				&weight_buffer.data[weight_buffer_view.byteOffset + weight_accessor.byteOffset]);

		vertices.resize(vertices.size() + position_accessor.count);
		for (size_t i = 0; i < position_accessor.count; i++) {
			SkinnedVertex vertex = {};
			vertex.position = glm::vec3(position_data[i * 3], position_data[i * 3 + 1], position_data[i * 3 + 2]);
			vertex.normal = glm::vec3(normal_data[i * 3], normal_data[i * 3 + 1], normal_data[i * 3 + 2]);
			vertex.uv = glm::vec2(uv_data[i * 2], uv_data[i * 2 + 1]);
			vertex.bone_ids =
					glm::ivec4(bone_data[i * 4], bone_data[i * 4 + 1], bone_data[i * 4 + 2], bone_data[i * 4 + 3]);
			vertex.weights = glm::vec4(
					weight_data[i * 4], weight_data[i * 4 + 1], weight_data[i * 4 + 2], weight_data[i * 4 + 3]);
			vertices.push_back(vertex);
		}

		// Load indices
		const auto &index_accessor = model.accessors[primitive.indices];
		const auto &index_buffer_view = model.bufferViews[index_accessor.bufferView];
		const auto &gltf_index_buffer = model.buffers[index_buffer_view.buffer];

		const auto &index_data = &gltf_index_buffer.data[index_buffer_view.byteOffset + index_accessor.byteOffset];

		indices.resize(indices.size() + index_accessor.count);
		for (size_t i = 0; i < index_accessor.count; i++) {
			switch (index_accessor.componentType) {
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
					indices.push_back(reinterpret_cast<const uint32_t *>(index_data)[i]);
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
					indices.push_back(reinterpret_cast<const uint16_t *>(index_data)[i]);
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
					indices.push_back(reinterpret_cast<const uint8_t *>(index_data)[i]);
					break;
				}
				default:
					break;
			}
		}

		meshes.emplace_back(vertices, indices);
	}
}

void SkinnedModel::process_skin(const tinygltf::Skin &skin) {
	joint_map.reserve(skin.joints.size());

	const tinygltf::Accessor &matrices_accessor = model.accessors[skin.inverseBindMatrices];
	const tinygltf::BufferView &matrices_buffer_view = model.bufferViews[matrices_accessor.bufferView];
	const tinygltf::Buffer &matrices_buffer = model.buffers[matrices_buffer_view.buffer];

	const float *matrices_data = reinterpret_cast<const float *>(
			&matrices_buffer.data[matrices_buffer_view.byteOffset + matrices_accessor.byteOffset]);

	for (int32_t i = 0; i < matrices_accessor.count; ++i) {
		Joint joint{};
		joint.id = skin.joints[i];

		glm::vec3 pos = Joint::get_glm_pos(matrices_data, i * 16);
		glm::quat rot = Joint::get_glm_rot(matrices_data, i * 16);

		uint16_t comp_rot[3];
		uint16_t comp_pos[3];

		Joint::vec4_to_uint16(glm::vec4(pos, 1.0f), comp_pos);
		Joint::quat_to_uint16(rot, comp_rot);

		joint.position[0] = comp_pos[0];
		joint.position[1] = comp_pos[1];
		joint.position[2] = comp_pos[2];

		joint.rotation[0] = comp_rot[0];
		joint.rotation[1] = comp_rot[1];
		joint.rotation[2] = comp_rot[2];

		joint_map[model.nodes[joint.id].name] = joint;
	}
}
