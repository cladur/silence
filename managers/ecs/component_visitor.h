#ifndef SILENCE_COMPONENT_VISITOR_H
#define SILENCE_COMPONENT_VISITOR_H

#include "types.h"
#include "world.h"

class ComponentVisitor {
public:
	static void visit(World &world, Entity entity, serialization::variant_type &variant) {
		// std::visit pass type to component in lambda from variant
		std::visit(
				[&](auto &component) {
					if (!world.has_component(entity, component)) {
						world.add_component(entity, component);
					}
					world.update_component(entity, component);
				},
				variant);
	}
};

#endif //SILENCE_COMPONENT_VISITOR_H
