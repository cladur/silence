#ifndef SILENCE_QUERY_BUILDER_H
#define SILENCE_QUERY_BUILDER_H
#include "ecs/world.h"

class QueryBuilder {
public:
	Signature blacklist;
	Signature whitelist;
	Entity parent = 0;
	World *world;

	void start(World &world) {
		this->world = &world;
	}

	QueryBuilder with_component(ComponentType component_type) {
		whitelist.set(component_type);
		return *this;
	}

	QueryBuilder without_component(ComponentType component_type) {
		blacklist.set(component_type);
		return *this;
	}

	// QueryBuilder with_parent(Entity parent) {
	// 	whitelist.set(world->get_component_type<Parent>);
	// 	this->parent = parent;
	// }

	// std::vector<Entity> build() {
	// 	std::vector<Entity> entities;
	// 	for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
	// 		for (ComponentType component_type = 0; component_type < MAX_COMPONENTS; component_type++) {

	//     }
	// }
};

#endif