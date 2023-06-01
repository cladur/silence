#ifndef SILENCE_ANIMATION_INSTANCE_H
#define SILENCE_ANIMATION_INSTANCE_H

#include "managers/render/ecs/model_instance.h"
#include "render/common/animation.h"
#include "render/common/material.h"

struct AnimationInstance {
public:
	Handle<Animation> animation_handle;
	float current_time = 0.0f;
	float ticks_per_second = 1000;
	bool is_looping = true;
	bool is_freeze = false;

	AnimationInstance();
	explicit AnimationInstance(const char *path);

	void serialize_json(nlohmann::json &j);
	void deserialize_json(nlohmann::json &j);
};

#endif //SILENCE_ANIMATION_INSTANCE_H
