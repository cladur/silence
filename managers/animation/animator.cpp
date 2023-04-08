#include "animator.h"
#include "animation.h"
#include "joint.h"

Animator::Animator(Animation &animation) {
	current_time = 0.0f;
	current_animation = &animation;

	bone_matrices.reserve(MAX_BONE_COUNT);

	for (int32_t i = 0; i < MAX_BONE_COUNT; ++i) {
		bone_matrices.emplace_back(1.0f);
	}
}
void Animator::update_animation(float dt) {
	delta_time = dt;
	if (current_animation) {
		current_time += static_cast<float>(current_animation->get_ticks_per_second()) * dt;

		current_time = fmod(current_time, static_cast<float>(current_animation->get_duration()));
		calculate_bone_transform(&current_animation->get_root_node(), glm::mat4(1.0f));
	}
}

void Animator::calculate_bone_transform(const HierarchyData *node, glm::mat4 parent_transform) {
	const std::string &node_name = node->name;

	glm::vec3 pos_decomp = Joint::uint16_to_vec4(node->position);
	glm::quat rot_decomp = Joint::uint16_to_quat(node->rotation);

	glm::mat4 node_transform = glm::translate(glm::mat4(1.0f), pos_decomp);
	node_transform *= glm::toMat4(rot_decomp);

	Bone *bone = current_animation->find_bone(node_name);

	if (bone) {
		bone->update(current_time);
		node_transform = bone->get_local_transform();
	}

	glm::mat4 global_transform = parent_transform * node_transform;

	std::unordered_map<std::string, Joint> joint_map = current_animation->get_joint_map();
	if (joint_map.find(node_name) != joint_map.end()) {
		int32_t index = joint_map[node_name].id;

		pos_decomp = Joint::uint16_to_vec4(joint_map[node_name].position);
		rot_decomp = Joint::uint16_to_quat(joint_map[node_name].rotation);

		glm::mat4 offset = glm::translate(glm::mat4(1.0f), pos_decomp);
		offset *= glm::toMat4(rot_decomp);

		bone_matrices[index] = global_transform * offset;
	}

	for (const HierarchyData &child : node->children) {
		calculate_bone_transform(&child, global_transform);
	}
}

void Animator::change_animation(Animation *animation) {
	current_animation = animation;
	current_time = 0.0f;
}
const std::vector<glm::mat4> &Animator::get_bone_matrices() const {
	return bone_matrices;
}
