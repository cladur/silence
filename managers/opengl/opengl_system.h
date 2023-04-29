#ifndef SILENCE_OPENGL_SYSTEM_H
#define SILENCE_OPENGL_SYSTEM_H

#include "ecs/systems/base_system.h"

class OpenglManager;

class OpenglSystem : public BaseSystem {
public:
	void startup();
	void update();
};

#endif //SILENCE_OPENGL_SYSTEM_H
