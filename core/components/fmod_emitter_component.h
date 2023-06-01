#ifndef SILENCE_FMOD_EMITTER_COMPONENT_H
#define SILENCE_FMOD_EMITTER_COMPONENT_H

#include <fmod_studio.hpp>
#include <nlohmann/json.hpp>
#include <string>
struct FMODEmitter {
	std::string event_path;
	bool is_3d = false;
	bool first_frame = true;
	FMOD::Studio::EventInstance *event_instance = nullptr;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["event_path"] = event_path;
		serialized_component["is_3d"] = is_3d;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "FMODEmitter";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		event_path = serialized_component["event_path"];
		is_3d = serialized_component["is_3d"];
	}

	bool is_playing() {
		FMOD_STUDIO_PLAYBACK_STATE state;
		event_instance->getPlaybackState(&state);
		return state != FMOD_STUDIO_PLAYBACK_STOPPED;
	}
};

#endif //SILENCE_FMOD_EMITTER_COMPONENT_H
