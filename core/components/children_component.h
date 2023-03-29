#ifndef SILENCE_CHILDREN_COMPONENT_H
#define SILENCE_CHILDREN_COMPONENT_H

#include "../../core/types.h"
#include <array>
#include <cassert>
#include <cstdint>
struct Children {
	std::uint8_t children_count{};
	std::array<Entity, MAX_CHILDREN> children{};

	bool add_children(Entity entity) {
		if (children_count >= MAX_CHILDREN || get_children_index(entity) != -1) {
			return false;
		}

		children[children_count] = entity;
		children_count++;
		return true;
	}

	bool remove_children(Entity entity) {
		if (children_count == 0) {
			return false;
		}

		int children_index = get_children_index(entity);

		if (children_index != -1) {
			children[children_index] = children[children_count - 1];
			children[children_count - 1] = 0;
			children_count--;
			return true;
		}

		return false;
	}

	int get_children_index(Entity entity) {
		auto entity_iterator = std::find(children.begin(), children.end(), entity);

		if (entity_iterator != std::end(children)) {
			return entity_iterator - children.begin();
		} else {
			return -1;
		}
	}
};

#endif //SILENCE_CHILDREN_COMPONENT_H
