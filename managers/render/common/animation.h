#ifndef SILENCE_ANIMATION_H
#define SILENCE_ANIMATION_H

#include "channel.h"
class SkinnedModel;

class Animation {
public:
	Animation() = default;

	void load_from_asset(const char *path);
	int32_t get_ticks_per_second() const;
	float get_duration() const;
	std::string name;

	std::unordered_map<std::string, Channel> channels;

private:
	const float SECONDS_TO_MS = 1000.0f;

	float duration_ms;
	int32_t ticks_per_second;
};

#endif //SILENCE_ANIMATION_H
