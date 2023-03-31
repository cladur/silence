#ifndef SILENCE_PARENT_COMPONENT_H
#define SILENCE_PARENT_COMPONENT_H

#include "../../core/types.h"
struct Parent {
	Entity parent;

	void set_parent(Entity entity) {
		if (entity == parent) {
			return;
		}

		parent = entity;
	}
};

#endif //SILENCE_PARENT_COMPONENT_H