#ifndef SILENCE_BONE_H
#define SILENCE_BONE_H

#include "animation_asset.h"

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

class Channel {
public:
	Channel(const assets::NodeAnimation &node, std::string bone_name, int32_t id);

	void update(float animation_time);

	glm::mat4 get_local_transform();
	std::string get_bone_name() const;
	int32_t get_bone_id() const;

	int32_t get_position_index(float animation_time);
	int32_t get_rotation_index(float animation_time);

private:
	float get_scale_factor(float last_time_stamp, float next_time_stamp, float animation_time);

	glm::mat4 interpolate_position(float animation_time);
	glm::mat4 interpolate_rotation(float animation_time);

	std::vector<KeyPosition> positions;
	std::vector<KeyRotation> rotations;

	glm::mat4 local_transform;
	std::string bone_name;
	int32_t id;
};

#endif //SILENCE_BONE_H
