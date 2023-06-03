#ifndef SILENCE_HIGHLIGHT_H
#define SILENCE_HIGHLIGHT_H

struct Highlight {
public:
	bool highlighted = false;

	void serialize_json(nlohmann::json &serialized_scene) {
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		highlighted = false;
	}
};

#endif //SILENCE_HIGHLIGHT_H