#ifndef SILENCE_CHECKPOINT_COMPONENT_H
#define SILENCE_CHECKPOINT_COMPONENT_H

struct Checkpoint {
	Entity player_collider = 0;
	// Things that need to be reset
	Entity enemy_collider = 0;
	Entity agent_spawn_pos = 0;
	Entity hacker_spawn_pos = 0;

	std::unordered_set<Entity> entities_to_reset;

	bool reached = false;
	bool queried_enemies = false;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["player_collider"] = player_collider;
		serialized_component["enemy_collider"] = enemy_collider;
		serialized_component["agent_spawn_pos"] = agent_spawn_pos;
		serialized_component["hacker_spawn_pos"] = hacker_spawn_pos;

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Checkpoint";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		if (serialized_component.contains("player_collider")) {
			player_collider = serialized_component["player_collider"];
		}
		if (serialized_component.contains("enemy_collider")) {
			enemy_collider = serialized_component["enemy_collider"];
		}
		if (serialized_component.contains("agent_spawn_pos")) {
			agent_spawn_pos = serialized_component["agent_spawn_pos"];
		}
		if (serialized_component.contains("hacker_spawn_pos")) {
			hacker_spawn_pos = serialized_component["hacker_spawn_pos"];
		}
	}
};

#endif //SILENCE_CHECKPOINT_COMPONENT_H
