#ifndef SILENCE_COMPONENT_VISITOR_H
#define SILENCE_COMPONENT_VISITOR_H

#include "ecs_manager.h"
#include "types.h"

class ComponentVisitor {
public:
	static void visit(Entity entity, serialization::variant_type &variant) {
		ECSManager &ecs_manager = ECSManager::get();
		// std::visit pass type to component in lambda from variant
		std::visit(
				[&](auto &component) {
					if (!ecs_manager.has_component(entity, component)) {
						ecs_manager.add_component(entity, component);
					}
					ecs_manager.update_component(entity, component);
				},
				variant);
	}
};

#endif //SILENCE_COMPONENT_VISITOR_H
