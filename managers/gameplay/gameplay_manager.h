#ifndef SILENCE_GAMEPLAY_MANAGER_H
#define SILENCE_GAMEPLAY_MANAGER_H

struct Scene;

class GameplayManager {
	uint32_t agent_entity = 0;
	uint32_t hacker_entity = 0;
public:
	static GameplayManager &get();

	void startup(Scene *scene);
	void shutdown();
	void update();

	glm::vec3 get_agent_position(Scene *scene) const;
	glm::vec3 get_hacker_position(Scene *scene) const;
};

#endif //SILENCE_GAMEPLAY_MANAGER_H
