#ifndef SILENCE_ENTITY_MANAGER_H
#define SILENCE_ENTITY_MANAGER_H

class EntityManager {
private:
	std::queue<Entity> available_entities{};
	std::array<Signature, MAX_ENTITIES> signatures{};
	uint32_t living_entities_count{};

public:
	void startup();
	void shutdown();

	Entity create_entity();
	void destroy_entity(Entity entity);
	void set_entity_signature(Entity entity, Signature signature);
	Signature get_signature(Entity entity);
};

#endif //SILENCE_ENTITY_MANAGER_H