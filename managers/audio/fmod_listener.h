#ifndef SILENCE_FMOD_LISTENER_H
#define SILENCE_FMOD_LISTENER_H

#include "fmod_studio.hpp"
#include "../systems/base_system.h"

class FmodListenerSystem : public BaseSystem {
public:
	void startup();

	void late_start();

	void update(float dt);
};

#endif //SILENCE_FMOD_LISTENER_H
