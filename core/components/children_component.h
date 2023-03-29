#ifndef SILENCE_CHILDREN_COMPONENT_H
#define SILENCE_CHILDREN_COMPONENT_H

#include "../../core/types.h"
#include <array>
#include <cassert>
#include <cstdint>
struct Children {
	std::uint8_t children_count{};
	std::array<Entity, MAX_CHILDREN> children{};

	void add_children(Entity entity) {
		if (children_count >= MAX_CHILDREN || has_children(entity) != -1) {
			return;
		}

		children[children_count] = entity;
		children_count++;
	}

	void remove_children(Entity entity) {
		if (children_count == 0) {
			return;
		}

		int children_index = has_children(entity);

		if (children_index != -1) {
			children[children_index] = children[children_count - 1];
			children[children_count - 1] = 0;
			children_count--;
		}
	}

	int has_children(Entity entity) {
		auto entity_iterator = std::find(children.begin(), children.end(), entity);

		if (entity_iterator != std::end(children)) {
			return entity_iterator - children.begin();
		} else {
			return -1;
		}
	}
};

#endif //SILENCE_CHILDREN_COMPONENT_H
