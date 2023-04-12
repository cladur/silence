#ifndef SILENCE_BONE_H
#define SILENCE_BONE_H

#include <tiny_gltf.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct AnimNode {
	std::vector<glm::vec3> translations;
	std::vector<glm::quat> rotations;
	std::vector<float> translation_times;
	std::vector<float> rotation_times;
	std::string node_name;
};

struct KeyPosition {
	//	uint16_t position[3];
	glm::vec3 position;
	float time_stamp;
};

struct KeyRotation {
	//	uint16_t rotation[3];
	glm::quat rotation;
	float time_stamp;
};

class Bone {
public:
	Bone(const AnimNode &node, int32_t id);

	void update(float animation_time);

	glm::mat4 get_local_transform();
	std::string get_bone_name() const;
	int32_t get_bone_id();

	int32_t get_position_index(float animationTime);
	int32_t get_rotation_index(float animationTime);

private:
	float get_scale_factor(float last_time_stamp, float next_time_stamp, float animation_time);

	glm::mat4 interpolate_position(float animation_time);
	glm::mat4 interpolate_rotation(float animation_time);

	std::vector<KeyPosition> positions;
	std::vector<KeyRotation> rotations;

	glm::mat4 local_transform;
	std::string name;
	int32_t id;
};

#endif //SILENCE_BONE_H
