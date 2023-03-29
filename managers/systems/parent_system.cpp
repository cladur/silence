#include "parent_system.h"
#include "../../core/components/children_component.h"
#include "../../core/components/parent_component.h"
#include "ecs/ecs_manager.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <iostream>

extern ECSManager ecs_manager;

void ParentSystem::startup() {
}
void ParentSystem::update(float dt) {
	for (auto const &entity : entities) {
		auto &children = ecs_manager.get_component<Children>(entity);

		SPDLOG_INFO("Children of {}", entity);
		for (int i = 0; i < children.children_count; i++) {
			std::cout << children.children[i] << " ";
		}

		std::cout << std::endl;
	}
}
