#include "particle_manager.h"

// some helpful math.
// x^2
float q_lerp(float x) {
	return (float)glm::pow(x, 2);
};

// 1 - (1 - x)^3, not true logarithmic but has a similiar curve
float l_lerp( float x) {
	return (float)(1.0f - glm::pow(1.0f - x, 3));
};

ParticleManager &ParticleManager::get() {
	static ParticleManager instance;
	return instance;
}

void ParticleManager::startup() {
	// inialize default particle vertices
	ParticleVertex v;
	v.position = glm::vec3(-0.5f, -0.5f, 0.0f);
	v.tex_coords = glm::vec2(0.0f, 0.0f);
	v.color = glm::vec4(1.0f);
	default_particle_vertices[0] = v;
	v.position = glm::vec3(0.5f, -0.5f, 0.0f);
	v.tex_coords = glm::vec2(1.0f, 0.0f);
	v.color = glm::vec4(1.0f);
	default_particle_vertices[1] = v;
	v.position = glm::vec3(0.5f, 0.5f, 0.0f);
	v.tex_coords = glm::vec2(1.0f, 1.0f);
	v.color = glm::vec4(1.0f);
	default_particle_vertices[2] = v;
	v.position = glm::vec3(-0.5f, 0.5f, 0.0f);
	v.tex_coords = glm::vec2(0.0f, 1.0f);
	v.color = glm::vec4(1.0f);
	default_particle_vertices[3] = v;
}

void ParticleManager::shutdown() {
}

void ParticleManager::update(float dt) {
	//update all active particles
	for (auto &entity : particles) {
		auto &particle_datas = entity.second;
		for (auto &particle : particle_datas) {
			if (!particle.active) { continue; }

			particle.lifetime = glm::max(0.0f, particle.lifetime);

			// returns a value between 0 and 1, meaning 0 is the beginning of the lifetime and 1 is the end
			float x = 1.0f - particle.life_left / particle.lifetime;
			if (particle.active) {
				// POSITION + VELOCITY
				switch (particle.velocity_transition_type) {
					case TransitionType::LINEAR:
						// linear
						particle.position += glm::lerp(
													 particle.velocity_begin,
													 particle.velocity_end,
													 x
													 ) * dt;
						break;
					case TransitionType::QUADRATIC:

						particle.position += glm::lerp(
													 particle.velocity_begin,
													 particle.velocity_end,
													 q_lerp(x)
															 ) * dt;
						break;
					case TransitionType::LOGARITHMIC:

						particle.position += glm::lerp(
													 particle.velocity_begin,
													 particle.velocity_end,
													 l_lerp(x)
															 ) * dt;
						break;
				}

				// COLOR
				glm::vec4 col;
				switch (particle.color_transition_type) {
					case TransitionType::LINEAR:
						col = glm::lerp(
								particle.color_begin,
								particle.color_end,
								x
						);
						break;
					case TransitionType::QUADRATIC:
						col = glm::lerp(
								particle.color_begin,
								particle.color_end,
								q_lerp(x)
						);
						break;
					case TransitionType::LOGARITHMIC:
						col = glm::lerp(
								particle.color_begin,
								particle.color_end,
								l_lerp(x)
						);
						break;
				}
				particle.color = col;

				// SIZE
				float size;
				switch (particle.size_transition_type) {
					case TransitionType::LINEAR:
						size = glm::lerp(
								particle.size_begin,
								particle.size_end,
								x
						);
						break;
					case TransitionType::QUADRATIC:
						size = glm::lerp(
								particle.size_begin,
								particle.size_end,
								q_lerp(x)
						);
						break;
					case TransitionType::LOGARITHMIC:
						size = glm::lerp(
								particle.size_begin,
								particle.size_end,
								l_lerp(x)
						);
						break;
				}
				particle.size = size;

				float rotation;
				switch (particle.rotation_transition_type) {
					case TransitionType::LINEAR:
						rotation = glm::lerp(
								particle.rotation_begin,
								particle.rotation_end,
								x
						);
						break;
					case TransitionType::QUADRATIC:
						rotation = glm::lerp(
								particle.rotation_begin,
								particle.rotation_end,
								q_lerp(x)
						);
						break;
					case TransitionType::LOGARITHMIC:
						rotation = glm::lerp(
								particle.rotation_begin,
								particle.rotation_end,
								l_lerp(x)
						);
						break;
				}
				particle.rotation = rotation;

				particle.life_left -= dt;
				if (particle.life_left <= 0.0f) {
					particle.active = false;
				}
			}
		}
	}
}

void ParticleManager::emit(uint32_t entity, ParticleData &particle) {
	particles[entity][particle_idx[entity]] = particle;
	particle_idx[entity] = ++particle_idx[entity] % particles[entity].size();
}
