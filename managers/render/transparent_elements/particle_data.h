#ifndef SILENCE_PARTICLE_DATA_H
#define SILENCE_PARTICLE_DATA_H

#include "components/particle_emitter_component.h"
#include "render/common/texture.h"
#include "resource/resource_manager.h"
#include <glm/glm.hpp>

#ifndef MAX_PARTICLES_PER_ENTITY
#define MAX_PARTICLES_PER_ENTITY 256
#endif //MAX_PARTICLES_PER_ENTITY

struct ParticleVertex {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec4 color = glm::vec4(1.0f);
	glm::vec2 tex_coords = glm::vec2(0.0f);
	float size = 1.0f;
};

struct ParticleSSBODataBlock {
	glm::vec4 position[MAX_PARTICLES_PER_ENTITY];
	glm::mat4 rotation[MAX_PARTICLES_PER_ENTITY];
	//glm::ivec4 tex_idxs[1000]; dont need it since we're rendering batched per-entity
	glm::vec4 colors[MAX_PARTICLES_PER_ENTITY];
};

struct ParticlePerEntityData {
	glm::vec3 entity_position;
	Handle<Texture> tex;
	bool is_textured = false;

	ParticlePerEntityData() = default;

	ParticlePerEntityData(ParticleEmitter &p, glm::vec3 entity_position) {
		this->entity_position = entity_position;
		if (p.is_textured) {
			is_textured = true;
			tex = p.texture;
		}
	}
};

struct ParticleData {
	glm::vec3 position, velocity_begin, velocity_end;
	TransitionType velocity_transition_type = TransitionType::LINEAR;
	glm::vec4 color, color_begin, color_end;
	TransitionType color_transition_type = TransitionType::LINEAR;
	float size, size_begin, size_end;
	TransitionType size_transition_type = TransitionType::LINEAR;
	float lifetime = 1.0f;
	float life_left = 1.0f;
	float rotation, rotation_begin, rotation_end;
	TransitionType rotation_transition_type = TransitionType::LINEAR;
	bool active = false;

	ParticleData() = default;

	ParticleData(ParticleEmitter &p, glm::vec3 position) {
		this->position = position + glm::vec3(
											((float)(rand() % 1000) / 1000.0f - 0.5f) * p.position_variance.x,
											((float)(rand() % 1000) / 1000.0f - 0.5f) * p.position_variance.y,
											((float)(rand() % 1000) / 1000.0f - 0.5f) * p.position_variance.z
									);
		velocity_begin = p.velocity_begin;
		velocity_end = p.velocity_end;
		velocity_transition_type = p.velocity_transition;
		color_begin = p.color_begin;
		color_end = p.color_end;
		color_transition_type = p.color_transition;
		size = p.size_begin;
		size_begin = p.size_begin;
		size_end = p.size_end;
		size_transition_type = p.size_transition;
		lifetime = p.lifetime;
		life_left = p.lifetime;
		rotation_begin = p.rotation_begin;
		rotation_end = p.rotation_end;
		rotation_transition_type = p.rotation_transition;
		this->active = true;
	}
};

#endif //SILENCE_PARTICLE_DATA_H
