#ifndef SILENCE_ANIMATION_H
#define SILENCE_ANIMATION_H

#include "channel.h"
#include <tiny_gltf.h>
class SkinnedModel;
class Bone;
struct NodeAnimation;

class Animation {
public:
	Animation() = default;

	void load_from_asset(const char *path);
	int32_t get_ticks_per_second() const;
	double get_duration() const;

private:
	const float SECONDS_TO_MS = 1000.0f;

	std::string name;
	float duration_ms;
	int32_t ticks_per_second;
	std::vector<Channel> channels;
};

#endif //SILENCE_ANIMATION_H
