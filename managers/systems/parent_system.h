#ifndef SILENCE_PARENT_SYSTEM_H
#define SILENCE_PARENT_SYSTEM_H

#include "ecs/base_system.h"
class ParentSystem : public BaseSystem {
public:
	void startup();
	void update(float dt);
};

#endif //SILENCE_PARENT_SYSTEM_H
