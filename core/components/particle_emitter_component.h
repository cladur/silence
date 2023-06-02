#ifndef SILENCE_PARTICLE_EMITTER_COMPONENT_H
#define SILENCE_PARTICLE_EMITTER_COMPONENT_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <nlohmann/json.hpp>

enum class TransitionType {
	LINEAR,
	QUADRATIC,
	LOGARITHMIC,
};

struct ParticleEmitter {
public:
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 velocity_begin, velocity_end;
	TransitionType velocity_transition = TransitionType::LINEAR;
	glm::vec4 color_begin = glm::vec4(1.0f);
	glm::vec4 color_end = glm::vec4(1.0f);
	TransitionType color_transition = TransitionType::LINEAR;
	float size_begin, size_end;
	TransitionType size_transition = TransitionType::LINEAR;
	float rotation_begin, rotation_end;
	TransitionType rotation_transition = TransitionType::LINEAR;
	float lifetime = 0.0f;

	void serialize_json(nlohmann::json &serialized_scene) {
		serialized_scene["position"] = {position.x, position.y, position.z};
		serialized_scene["velocity_begin"] = {velocity_begin.x, velocity_begin.y, velocity_begin.z};
		serialized_scene["velocity_end"] = {velocity_end.x, velocity_end.y, velocity_end.z};
		serialized_scene["velocity_transition"] = static_cast<int>(velocity_transition);
		serialized_scene["color_begin"] = {color_begin.r, color_begin.g, color_begin.b, color_begin.a};
		serialized_scene["color_end"] = {color_end.r, color_end.g, color_end.b, color_end.a};
		serialized_scene["color_transition"] = static_cast<int>(color_transition);
		serialized_scene["size_begin"] = size_begin;
		serialized_scene["size_end"] = size_end;
		serialized_scene["size_transition"] = static_cast<int>(size_transition);
		serialized_scene["rotation_begin"] = rotation_begin;
		serialized_scene["rotation_end"] = rotation_end;
		serialized_scene["rotation_transition"] = static_cast<int>(rotation_transition);
		serialized_scene["lifetime"] = lifetime;
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		position = glm::vec3(serialized_component["position"][0], serialized_component["position"][1], serialized_component["position"][2]);
		velocity_begin = glm::vec3(serialized_component["velocity_begin"][0], serialized_component["velocity_begin"][1], serialized_component["velocity_begin"][2]);
		velocity_end = glm::vec3(serialized_component["velocity_end"][0], serialized_component["velocity_end"][1], serialized_component["velocity_end"][2]);
		velocity_transition = static_cast<TransitionType>(serialized_component["velocity_transition"]);
		color_begin = glm::vec4(serialized_component["color_begin"][0], serialized_component["color_begin"][1], serialized_component["color_begin"][2], serialized_component["color_begin"][3]);
		color_end = glm::vec4(serialized_component["color_end"][0], serialized_component["color_end"][1], serialized_component["color_end"][2], serialized_component["color_end"][3]);
		color_transition = static_cast<TransitionType>(serialized_component["color_transition"]);
		size_begin = serialized_component["size_begin"];
		size_end = serialized_component["size_end"];
		size_transition = static_cast<TransitionType>(serialized_component["size_transition"]);
		rotation_begin = serialized_component["rotation_begin"];
		rotation_end = serialized_component["rotation_end"];
		rotation_transition = static_cast<TransitionType>(serialized_component["rotation_transition"]);
		lifetime = serialized_component["lifetime"];
	}
};

#endif //SILENCE_PARTICLE_EMITTER_COMPONENT_H
