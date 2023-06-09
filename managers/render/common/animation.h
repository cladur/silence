#ifndef SILENCE_ANIMATION_H
#define SILENCE_ANIMATION_H

#include "animation/pose.h"
#include "channel.h"
class SkinnedModel;
struct AnimData;

class Animation {
public:
	Animation() = default;

	void load_from_asset(const char *path);
	float get_duration() const;
	std::string name;

	void sample(AnimData &data, Pose &result);
	std::unordered_map<std::string, Channel> channels;

private:
	const float SECONDS_TO_MS = 1000.0f;

	float duration_ms;
};

#endif //SILENCE_ANIMATION_H
