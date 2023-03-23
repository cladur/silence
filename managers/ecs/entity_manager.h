#include "../../core/types.h"
#include <array>
#include <queue>

#ifndef SILENCE_ENTITY_MANAGER_H
#define SILENCE_ENTITY_MANAGER_H

#endif //SILENCE_ENTITY_MANAGER_H

class EntityManager {
private:
	std::queue<Entity> available_entities{};
	std::array<Signature, MAX_ENTITIES> signatures{};
	uint32_t living_entities_count{};

public:
	bool startup();
	void shutdown();

	Entity createEntity();
	void destroyEntity(Entity entity);
	void setSignature(Entity entity, Signature signature);
	Signature getSignature(Entity entity);
};