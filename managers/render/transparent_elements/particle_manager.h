#ifndef SILENCE_PARTICLE_MANAGER_H
#define SILENCE_PARTICLE_MANAGER_H

#include "particle_data.h"

#define MAX_PARTICLES_PER_ENTITY 256

class ParticleManager {
public:

	// map of entity id to pair of array of particle data and entity position
	std::map<uint32_t, std::pair<std::array<ParticleData, MAX_PARTICLES_PER_ENTITY>, glm::vec3>> particles;
	std::map<uint32_t, uint32_t> particle_idx;
	static ParticleManager &get();
	ParticleVertex default_particle_vertices[4];

	void startup();
	void shutdown();
	void emit(uint32_t entity, ParticleData &particle);
	void update(float dt);
};

#endif //SILENCE_PARTICLE_MANAGER_H
