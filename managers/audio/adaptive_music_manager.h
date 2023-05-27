#ifndef SILENCE_ADAPTIVE_MUSIC_MANAGER_H
#define SILENCE_ADAPTIVE_MUSIC_MANAGER_H

#include "event_reference.h"
#include "fmod_studio.hpp"

class AdaptiveMusicManager {
private:
	EventReference event_ref;
	FMOD::Studio::EventInstance *event_instance = nullptr;
	const char* detection_param = "detection_level";
	const char* drum_param = "drum_intensity";
	const char* intensity_param = "intensity";
	const char* lpf = "music_master_filter";

	float drum_int_calc_val = 2.3f;

public:
	static AdaptiveMusicManager &get();

	void startup(std::string name_of_event);
	void shutdown();
	void update(float dt);

	void play();
	void stop();
};

#endif //SILENCE_ADAPTIVE_MUSIC_MANAGER_H