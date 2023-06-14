#ifndef SILENCE_GAMEPLAY_MANAGER_H
#define SILENCE_GAMEPLAY_MANAGER_H

struct Scene;
class World;

class GameplayManager {
	bool disabled = true;
	uint32_t agent_entity = 0;
	uint32_t hacker_entity = 0;
	bool is_agent_crouching = false;
	float highest_detection = 0.0f;
	uint32_t enemies_near_player = 0;
	std::vector<uint32_t> enemy_entities;
	std::vector<float> detection_levels;

public:
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

	glm::vec3 get_agent_position(Scene *scene) const;
	glm::vec3 get_hacker_position(Scene *scene) const;
	uint32_t get_agent_entity() const;
	uint32_t get_hacker_entity() const;
	uint32_t get_agent_camera(Scene *scene) const;
	uint32_t get_hacker_camera(Scene *scene) const;
	float get_highest_detection() const;
	uint32_t get_enemies_near_player() const;
};

#endif //SILENCE_GAMEPLAY_MANAGER_H
