#ifndef SILENCE_ENEMY_DATA_H
#define SILENCE_ENEMY_DATA_H

#include "ai/state_machine/state_machine.h"
#include "ai/state_machine/states/enemy/enemy_dead.h"
#include "ai/state_machine/states/enemy/enemy_distracted.h"
#include "ai/state_machine/states/enemy/enemy_fully_aware.h"
#include "ai/state_machine/states/enemy/enemy_looking.h"
#include "ai/state_machine/states/enemy/enemy_patrolling.h"
#include "ai/state_machine/states/enemy/enemy_stationary_patrolling.h"

struct EnemyData {
	StateMachine state_machine;
	EnemyPatrolling patrolling_state;
	EnemyLooking looking_state;
	EnemyFullyAware fully_aware_state;
	EnemyStationaryPatrolling stationary_patrolling_state;
	EnemyDead dead_state;
	EnemyDistracted distracted_state;
	//UISlider detection_slider;
	bool first_frame = true;
	bool is_blinded = false;
	float detection_level = 0.0f;

	glm::vec3 distraction_target = glm::vec3(0.0f);
	float distraction_cooldown = 0.0f;

	DetectionTarget detection_target = DetectionTarget::NONE;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "EnemyData";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
	}
};

#endif //SILENCE_ENEMY_DATA_H
