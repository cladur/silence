#include "bone.h"
#include "joint.h"

Bone::Bone(const AnimNode &node, int32_t id) : name(node.node_name), id(id), local_transform(1.0f) {
	positions.reserve(node.translations.size());
	for (int32_t position_index = 0; position_index < node.translations.size(); ++position_index) {
		glm::vec3 pos = node.translations[position_index];

		//		uint16_t pos[3];
		//		Joint::vec4_to_uint16(glm::vec4(glm_pos, 1.0f), pos);

		float time_stamp = node.translation_times[position_index];
		KeyPosition data;

		//		data.position[0] = pos[0];
		//		data.position[1] = pos[1];
		//		data.position[2] = pos[2];
		data.position = pos;
		data.time_stamp = time_stamp;
		positions.push_back(data);
	}

	rotations.reserve(node.rotations.size());
	for (int32_t rotation_index = 0; rotation_index < node.rotations.size(); ++rotation_index) {
		glm::quat rot = node.rotations[rotation_index];
		//		uint16_t rot[3];
		//		Joint::quat_to_uint16(glm_rot, rot);
		float time_stamp = node.translation_times[rotation_index];

		KeyRotation data;

		//		data.rotation[0] = rot[0];
		//		data.rotation[1] = rot[1];
		//		data.rotation[2] = rot[2];

		data.rotation = rot;
		data.time_stamp = time_stamp;
		rotations.push_back(data);
	}
}

void Bone::update(float animation_time) {
	glm::mat4 translation = interpolate_position(animation_time);
	glm::mat4 rotation = interpolate_rotation(animation_time);
	local_transform = translation * rotation;
}

float Bone::get_scale_factor(float last_time_stamp, float next_time_stamp, float animation_time) {
	float mid_way_length = animation_time - last_time_stamp;
	float frames_diff = next_time_stamp - last_time_stamp;
	float scale_factor = mid_way_length / frames_diff;
	return scale_factor;
}

int32_t Bone::get_position_index(float animationTime) {
	for (int32_t index = 0; index < positions.size() - 1; ++index) {
		if (animationTime < positions[index + 1].time_stamp) {
			return index;
		}
	}
	return 0;
}

int32_t Bone::get_rotation_index(float animationTime) {
	for (int32_t index = 0; index < rotations.size() - 1; ++index) {
		if (animationTime < rotations[index + 1].time_stamp) {
			return index;
		}
	}
	return 0;
}

glm::mat4 Bone::interpolate_position(float animation_time) {
	if (positions.size() == 1) {
		return glm::translate(glm::mat4(1.0f), positions[0].position);
	}

	int32_t p0_index = get_position_index(animation_time);
	int32_t p1_index = p0_index + 1;
	float scale_factor =
			get_scale_factor(positions[p0_index].time_stamp, positions[p1_index].time_stamp, animation_time);
	glm::vec3 final_position = glm::mix(positions[p0_index].position, positions[p1_index].position, scale_factor);
	return glm::translate(glm::mat4(1.0f), final_position);
}

glm::mat4 Bone::interpolate_rotation(float animation_time) {
	if (rotations.size() == 1) {
		glm::quat rotation = glm::normalize(rotations[0].rotation);
		return glm::toMat4(rotation);
	}

	int32_t p0_index = get_rotation_index(animation_time);
	int32_t p1_index = p0_index + 1;
	float scale_factor =
			get_scale_factor(rotations[p0_index].time_stamp, rotations[p1_index].time_stamp, animation_time);
	glm::quat final_rotation = glm::slerp(rotations[p0_index].rotation, rotations[p1_index].rotation, scale_factor);
	final_rotation = glm::normalize(final_rotation);
	return glm::toMat4(final_rotation);
}

glm::mat4 Bone::get_local_transform() {
	return local_transform;
}

std::string Bone::get_bone_name() const {
	return name;
}

int32_t Bone::get_bone_id() {
	return id;
}