#ifndef SILENCE_BONE_H
#define SILENCE_BONE_H

#include "animation/pose.h"
#include "assets/animation_asset.h"

struct KeyPosition {
	glm::vec3 position;
	float time_stamp;
};

struct KeyRotation {
	glm::quat rotation;
	float time_stamp;
};

class Channel {
public:
	Channel(const assets::NodeAnimation &node);

	void update(float animation_time);

	int32_t get_position_index(float animation_time);
	int32_t get_rotation_index(float animation_time);

	std::vector<KeyPosition> positions;
	std::vector<KeyRotation> rotations;
	Xform local_transform;

private:
	float get_scale_factor(float last_time_stamp, float next_time_stamp, float animation_time);

	void interpolate_position(float animation_time, glm::vec3 &result);
	void interpolate_rotation(float animation_time, glm::quat &result);
};

#endif //SILENCE_BONE_H
