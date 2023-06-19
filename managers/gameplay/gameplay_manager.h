#ifndef SILENCE_GAMEPLAY_MANAGER_H
#define SILENCE_GAMEPLAY_MANAGER_H

#include "ecs/systems/agent_system.h"
#include "ecs/systems/hacker_system.h"

struct Scene;
class World;

enum class GameState {
	MAIN_MENU,
	GAME,
};

class GameplayManager {
	bool disabled = true;
	uint32_t agent_entity = 0;
	uint32_t hacker_entity = 0;
	bool is_agent_crouching = false;
	float highest_detection = 0.0f;
	uint32_t enemies_near_player = 0;
	std::vector<uint32_t> enemy_entities;
	std::vector<float> detection_levels;

	std::shared_ptr<AgentSystem> agent_system = nullptr;
	std::shared_ptr<HackerSystem> hacker_system = nullptr;

public:
	GameState game_state = GameState::MAIN_MENU;

	static std::default_random_engine random_generator;
	static GameplayManager &get();

	bool first_start = true;
	void startup(Scene *scene);
	void shutdown();
	void update(World &world, float dt);
	void enable();

	void add_enemy_entity(uint32_t entity);
	void add_detection_level(float detection_level);

	void set_agent_crouch(bool crouching);
	bool get_agent_crouch() const;

	void set_agent_system(std::shared_ptr<AgentSystem> agent_system);
	void set_hacker_system(std::shared_ptr<HackerSystem> hacker_system);

	std::shared_ptr<AgentSystem> get_agent_system();
	std::shared_ptr<HackerSystem> get_hacker_system();

	glm::vec3 get_agent_position(Scene *scene) const;
	glm::vec3 get_hacker_position(Scene *scene) const;
	uint32_t get_agent_entity() const;
	uint32_t get_hacker_entity() const;
	uint32_t get_agent_camera(Scene *scene) const;
	uint32_t get_hacker_camera(Scene *scene) const;
	float get_highest_detection() const;
	uint32_t get_enemies_near_player() const;

	uint32_t get_menu_camera(Scene *scene) const;
};

#endif //SILENCE_GAMEPLAY_MANAGER_H
