#ifndef SILENCE_PARTICLE_EMITTER_COMPONENT_H
#define SILENCE_PARTICLE_EMITTER_COMPONENT_H

#include "render/common/texture.h"
#include "resource/resource_manager.h"
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
	glm::vec3 position_variance = glm::vec3(0.2f);
	glm::vec3 velocity_begin, velocity_end;
	TransitionType velocity_transition = TransitionType::LINEAR;
	glm::vec4 color_begin = glm::vec4(1.0f);
	glm::vec4 color_end = glm::vec4(1.0f);
	TransitionType color_transition = TransitionType::LINEAR;
	float size_begin, size_end;
	TransitionType size_transition = TransitionType::LINEAR;
	float rotation_begin, rotation_end;
	TransitionType rotation_transition = TransitionType::LINEAR;
	float rate = 5.0f; // particles per second
	float lifetime = 0.0f;
	float particle_time = 0.0f;
	Handle<Texture> texture;
	bool is_textured = false;
	bool is_one_shot = false;
	float one_shot_duration = 0.5f;

	float oneshot_time_left = 0.0f;

	void serialize_json(nlohmann::json &serialized_scene) {
		auto &rm = ResourceManager::get();
		nlohmann::json::object_t serialized_component;
		serialized_component["position"] = {position.x, position.y, position.z};
		serialized_component["position_variance"] = {position_variance.x, position_variance.y, position_variance.z};
		serialized_component["velocity_begin"] = {velocity_begin.x, velocity_begin.y, velocity_begin.z};
		serialized_component["velocity_end"] = {velocity_end.x, velocity_end.y, velocity_end.z};
		serialized_component["velocity_transition"] = static_cast<int>(velocity_transition);
		serialized_component["color_begin"] = {color_begin.r, color_begin.g, color_begin.b, color_begin.a};
		serialized_component["color_end"] = {color_end.r, color_end.g, color_end.b, color_end.a};
		serialized_component["color_transition"] = static_cast<int>(color_transition);
		serialized_component["size_begin"] = size_begin;
		serialized_component["size_end"] = size_end;
		serialized_component["size_transition"] = static_cast<int>(size_transition);
		serialized_component["rotation_begin"] = rotation_begin;
		serialized_component["rotation_end"] = rotation_end;
		serialized_component["rotation_transition"] = static_cast<int>(rotation_transition);
		serialized_component["rate"] = rate;
		serialized_component["lifetime"] = lifetime;
		serialized_component["texture_name"] = rm.get_texture_name(texture);
		serialized_component["is_textured"] = is_textured;
		serialized_component["is_one_shot"] = is_one_shot;
		serialized_component["one_shot_duration"] = one_shot_duration;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "ParticleEmitter";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		auto &rm = ResourceManager::get();
		position = glm::vec3(serialized_component["position"][0], serialized_component["position"][1], serialized_component["position"][2]);
		position_variance = glm::vec3(serialized_component["position_variance"][0], serialized_component["position_variance"][1], serialized_component["position_variance"][2]);
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
		rate = serialized_component["rate"];
		lifetime = serialized_component["lifetime"];
		std::string tex_name = serialized_component["texture_name"];
		if (!tex_name.empty()) {
			texture = rm.load_texture(tex_name.c_str());
		}
		is_textured = serialized_component["is_textured"];
		is_one_shot = serialized_component["is_one_shot"];
		one_shot_duration = serialized_component["one_shot_duration"];
	}

	void trigger_oneshot() {
		if (is_one_shot) {
			oneshot_time_left = one_shot_duration;
		} else {
			SPDLOG_WARN("Tried to trigger oneshot on non-oneshot emitter.");
		}
	}
};

#endif //SILENCE_PARTICLE_EMITTER_COMPONENT_H
