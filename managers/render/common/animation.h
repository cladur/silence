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
	Channel *find_channel(const std::string &bone_name);
	std::string name;

private:
	const float SECONDS_TO_MS = 1000.0f;

	float duration_ms;
	int32_t ticks_per_second;
	std::vector<Channel> channels;
};

#endif //SILENCE_ANIMATION_H
