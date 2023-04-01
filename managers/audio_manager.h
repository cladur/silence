#ifndef SILENCE_AUDIO_MANAGER_H
#define SILENCE_AUDIO_MANAGER_H

#include "fmod_studio.hpp"

class AudioManager {
	FMOD::Studio::System *system = nullptr;

public:
	void startup();
	void shutdown();
	void update();
};

#endif //SILENCE_AUDIO_MANAGER_H
