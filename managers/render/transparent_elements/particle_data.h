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
	glm::vec4 up[MAX_PARTICLES_PER_ENTITY];
	glm::vec4 right[MAX_PARTICLES_PER_ENTITY];
};

struct ParticlePerEntityData {
	glm::vec3 entity_position;
	Handle<Texture> tex;
	bool is_textured = false;
	bool is_billboard = true;
	glm::vec3 non_billboard_right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 non_billboard_up = glm::vec3(0.0f, 1.0f, 0.0f);

	ParticlePerEntityData() = default;

	ParticlePerEntityData(ParticleEmitter &p, glm::vec3 entity_position, glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) {
		this->entity_position = entity_position;
		if (p.is_textured) {
			is_textured = true;
			tex = p.texture;
		}
		is_billboard = p.is_billboard;
		non_billboard_right = right;
		non_billboard_up = up;
	}
};

struct ParticleData {
	glm::vec3 position, velocity_begin, velocity_end, velocity_variance;
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

	ParticleData(ParticleEmitter &p, Transform &t, glm::vec3 pos) {
		glm::vec3 glob_pos = pos.x * t.get_global_right() + pos.y * t.get_global_up() + pos.z * t.get_global_forward();
		this->position = glob_pos + t.get_global_position() +
											(((float)(rand() % 1000) / 1000.0f - 0.5f) * p.position_variance.x * t.get_global_right()) +
											(((float)(rand() % 1000) / 1000.0f - 0.5f) * p.position_variance.y * t.get_global_up()) +
											(((float)(rand() % 1000) / 1000.0f - 0.5f) * p.position_variance.z * t.get_global_forward());
		this->velocity_variance =
				(((float)(rand() % 1000) / 1000.0f - 0.5f) * p.velocity_variance.x * t.get_global_right()) +
				(((float)(rand() % 1000) / 1000.0f - 0.5f) * p.velocity_variance.y * t.get_global_up()) +
				(((float)(rand() % 1000) / 1000.0f - 0.5f) * p.velocity_variance.z * t.get_global_forward());
		velocity_begin = p.velocity_begin.x * t.get_global_right() + p.velocity_begin.y * t.get_global_up() + p.velocity_begin.z * t.get_global_forward();
		velocity_begin += velocity_variance;
		velocity_end = p.velocity_end.x * t.get_global_right() + p.velocity_end.y * t.get_global_up() + p.velocity_end.z * t.get_global_forward();
		velocity_end += velocity_variance;
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
