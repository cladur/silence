#ifndef SILENCE_PARTICLE_MANAGER_H
#define SILENCE_PARTICLE_MANAGER_H

#include "particle_data.h"

#define MAX_PARTICLES_PER_ENTITY 256

class ParticleManager {
public:
	std::map<uint32_t, std::array<ParticleData, MAX_PARTICLES_PER_ENTITY>> particles;
	std::map<uint32_t, uint32_t> particle_idx;
	static ParticleManager &get();
	ParticleVertex default_particle_vertices[4];

	void startup();
	void shutdown();
	void emit(uint32_t entity, ParticleData &particle);
	void update(float dt);
};

#endif //SILENCE_PARTICLE_MANAGER_H
