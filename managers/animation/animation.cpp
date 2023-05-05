#include "animation.h"
#include "joint.h"
#include "skinned_model.h"

Animation::Animation(const char *animation_path) {
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool result = loader.LoadASCIIFromFile(&model, &err, &warn, animation_path);
	if (!result) {
		SPDLOG_ERROR("Failed to load gltf file: {} - {} - {}", animation_path, err, warn);
		return;
	}

	if (!warn.empty()) {
		SPDLOG_WARN("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		SPDLOG_ERROR("Err: %s\n", err.c_str());
		return;
	}

	if (model.animations.empty()) {
		SPDLOG_ERROR("Model does not have any animations: {}", animation_path);
		return;
	}

	// Read first animation
	tinygltf::Animation animation = model.animations[0];

	// Validate if file is fine
	if (animation.samplers.empty() || model.accessors.size() <= animation.samplers[0].input) {
		SPDLOG_ERROR("Animation has invalid samplers: {}", animation_path);
		return;
	}

	// Read animation duration and multiply to get milliseconds
	duration_ms = model.accessors[animation.samplers[0].input].maxValues[0] * SECONDS_TO_MS;

	// Set ticks to 1 per millisecond
	ticks_per_second = 1000;

	process_animation(animation);

	read_hierarchy_data(root_node, model.nodes[0]);

	bones.reserve(temp_anim_nodes.size());
	for (int32_t i = 0; i < temp_anim_nodes.size(); ++i) {
		if (!temp_anim_nodes[i].node_name.empty()) {
			load_bones(temp_anim_nodes[i], i);
		}
	}
	temp_anim_nodes.clear();

	for (const auto &bone : bones) {
		std::cout << bone.get_bone_name() << std::endl;
	}
}

void Animation::process_animation(const tinygltf::Animation &animation) {
	AnimNode empty;
	temp_anim_nodes.reserve(model.nodes.size());
	for (int32_t i = 0; i < model.nodes.size(); ++i) {
		temp_anim_nodes.push_back(empty);
	}

	for (const tinygltf::AnimationChannel &channel : animation.channels) {
		if (!channel.target_path.compare("scale")) { //Skip scale
			continue;
		}

		const tinygltf::AnimationSampler &sampler = animation.samplers[channel.sampler];

		const tinygltf::Accessor &times_accessor = model.accessors[sampler.input];
		const tinygltf::BufferView &times_buffer_view = model.bufferViews[times_accessor.bufferView];
		const tinygltf::Buffer &times_buffer = model.buffers[times_buffer_view.buffer];

		const tinygltf::Accessor &transform_accessor = model.accessors[sampler.output];
		const tinygltf::BufferView &transform_buffer_view = model.bufferViews[transform_accessor.bufferView];
		const tinygltf::Buffer &transform_buffer = model.buffers[transform_buffer_view.buffer];

		const float *times_data = reinterpret_cast<const float *>(
				&times_buffer.data[times_buffer_view.byteOffset + times_accessor.byteOffset]);

		const float *transform_data = reinterpret_cast<const float *>(
				&transform_buffer.data[transform_buffer_view.byteOffset + transform_accessor.byteOffset]);

		const int32_t node_index = channel.target_node;
		temp_anim_nodes[node_index].node_name = model.nodes[node_index].name;

		if (transform_accessor.type == TINYGLTF_TYPE_VEC3) { // Load translations
			temp_anim_nodes[node_index].translation_times.reserve(times_accessor.count);
			temp_anim_nodes[node_index].translations.reserve(transform_accessor.count);
			for (size_t i = 0; i < transform_accessor.count; ++i) {
				temp_anim_nodes[node_index].translation_times.push_back(times_data[i]);
				temp_anim_nodes[node_index].translations.emplace_back(
						transform_data[i * 3], transform_data[i * 3 + 1], transform_data[i * 3 + 2]);
			}
		} else if (transform_accessor.type == TINYGLTF_TYPE_VEC4) { // Load rotations
			temp_anim_nodes[node_index].rotation_times.reserve(times_accessor.count);
			temp_anim_nodes[node_index].rotations.reserve(transform_accessor.count);
			for (size_t i = 0; i < transform_accessor.count; ++i) {
				temp_anim_nodes[node_index].rotation_times.push_back(times_data[i]);
				temp_anim_nodes[node_index].rotations.emplace_back(transform_data[i * 4], transform_data[i * 4 + 1],
						transform_data[i * 4 + 2], transform_data[i * 4 + 3]);
			}
		} else {
			SPDLOG_WARN("Invalid transform type at node: {}", temp_anim_nodes[node_index].node_name);
		}
	}
}

void Animation::read_hierarchy_data(HierarchyData &dest, const tinygltf::Node &src) {
	bool is_node_bone = !src.translation.empty() && !src.rotation.empty();

	glm::vec3 pos = glm::vec3(0.0f);
	glm::quat rot = glm::quat();

	if (is_node_bone) {
		pos = Joint::get_glm_vec3(src.translation);
		rot = Joint::get_glm_quat(src.rotation);
	} else if (!src.matrix.empty()) {
		pos = Joint::get_glm_pos(src.matrix.data(), 0);
		rot = Joint::get_glm_rot(src.matrix.data(), 0);
	}

	dest.name = src.name;

	uint16_t pos_comp[3];
	Joint::vec4_to_uint16(glm::vec4(pos, 1.0f), pos_comp);
	uint16_t rot_comp[3];
	Joint::quat_to_uint16(rot, rot_comp);
	dest.position[0] = pos_comp[0];
	dest.position[1] = pos_comp[1];
	dest.position[2] = pos_comp[2];

	dest.rotation[0] = rot_comp[0];
	dest.rotation[1] = rot_comp[1];
	dest.rotation[2] = rot_comp[2];

	dest.children.reserve(src.children.size());

	for (int32_t i : src.children) {
		HierarchyData data;
		read_hierarchy_data(data, model.nodes[i]);
		dest.children.push_back(data);
	}
}

void Animation::load_bones(const AnimNode &node, int32_t id) {
	bones.emplace_back(node, id);
}

int32_t Animation::get_ticks_per_second() const {
	return ticks_per_second;
}

double Animation::get_duration() const {
	return duration_ms;
}

const HierarchyData &Animation::get_root_node() const {
	return root_node;
}

Bone *Animation::find_bone(const std::string &name) {
	auto iter =
			std::find_if(bones.begin(), bones.end(), [&](const Bone &Bone) { return Bone.get_bone_name() == name; });

	if (iter == bones.end()) {
		return nullptr;
	} else {
		return &(*iter);
	}
}
