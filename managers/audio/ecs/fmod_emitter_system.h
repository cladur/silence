#ifndef SILENCE_FMOD_EMITTER_SYSTEM_H
#define SILENCE_FMOD_EMITTER_SYSTEM_H

#include <ecs/systems/base_system.h>
class FMODEmitterSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_FMOD_EMITTER_SYSTEM_H
