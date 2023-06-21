#ifndef SILENCE_PARTICLE_RENDER_SYSTEM_H
#define SILENCE_PARTICLE_RENDER_SYSTEM_H

#include <ecs/systems/base_system.h>
class World;

class ParticleRenderSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_PARTICLE_RENDER_SYSTEM_H
