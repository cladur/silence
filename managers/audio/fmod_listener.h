#ifndef SILENCE_FMOD_LISTENER_H
#define SILENCE_FMOD_LISTENER_H

#include "ecs/systems/base_system.h"
#include "fmod_studio.hpp"

class FmodListenerSystem : public BaseSystem {
public:
	void startup();
	void update(float dt);
};

#endif //SILENCE_FMOD_LISTENER_H
