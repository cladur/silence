#ifndef SILENCE_SKINNED_MODEL_H
#define SILENCE_SKINNED_MODEL_H

#include "skinned_mesh.h"

struct Rig {
	std::vector<glm::vec3> positions;
	std::vector<glm::quat> rotations;
	std::unordered_map<std::string, uint64_t> names;
	std::vector<int64_t> parents;
};

class SkinnedModel {
public:
	void load_from_asset(const char *path);

	std::string name;
	glm::mat4 root{};

	std::vector<SkinnedMesh> meshes;
	Rig rig;

private:
	// model data
	std::string directory;
};

/*
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
*/
#endif //SILENCE_SKINNED_MODEL_H
