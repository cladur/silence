#ifndef SILENCE_COMPONENT_VISITOR_H
#define SILENCE_COMPONENT_VISITOR_H

#include "components/enemy_path_component.h"
#include "types.h"
#include "world.h"
#include <unordered_map>
#include <utility>

class ComponentVisitor {
private:
	static void update_interactable_ids(Interactable &interactable, std::unordered_map<Entity, Entity> id_map) {
		for (auto &target : interactable.interaction_targets) {
			if (id_map.find(target) != id_map.end()) {
				target = id_map[target];
			}
		}
		if (id_map.find(interactable.cable_parent) != id_map.end()) {
			interactable.cable_parent = id_map[interactable.cable_parent];
		}
	}

	static void update_enemy_path_ids(EnemyPath &enemy_path, std::unordered_map<Entity, Entity> id_map) {
		if (id_map.find(enemy_path.path_parent) != id_map.end()) {
			enemy_path.path_parent = id_map[enemy_path.path_parent];
		}
	}

	static void update_agent_data_ids(AgentData &agent_data, std::unordered_map<Entity, Entity> id_map) {
		if (id_map.find(agent_data.model) != id_map.end()) {
			agent_data.model = id_map[agent_data.model];
		}

		if (id_map.find(agent_data.camera) != id_map.end()) {
			agent_data.camera = id_map[agent_data.camera];
		}

		if (id_map.find(agent_data.camera_pivot) != id_map.end()) {
			agent_data.camera_pivot = id_map[agent_data.camera_pivot];
		}

		if (id_map.find(agent_data.camera_pivot_target) != id_map.end()) {
			agent_data.camera_pivot_target = id_map[agent_data.camera_pivot_target];
		}

		if (id_map.find(agent_data.spring_arm) != id_map.end()) {
			agent_data.spring_arm = id_map[agent_data.spring_arm];
		}
	}

	static void update_hacker_data_ids(HackerData &hacker_data, std::unordered_map<Entity, Entity> id_map) {
		if (id_map.find(hacker_data.model) != id_map.end()) {
			hacker_data.model = id_map[hacker_data.model];
		}

		if (id_map.find(hacker_data.camera) != id_map.end()) {
			hacker_data.camera = id_map[hacker_data.camera];
		}

		if (id_map.find(hacker_data.camera_pivot) != id_map.end()) {
			hacker_data.camera_pivot = id_map[hacker_data.camera_pivot];
		}

		if (id_map.find(hacker_data.scorpion_camera_transform) != id_map.end()) {
			hacker_data.scorpion_camera_transform = id_map[hacker_data.scorpion_camera_transform];
		}
	}

	static void update_detection_camera_ids(
			DetectionCamera &detection_camera, std::unordered_map<Entity, Entity> id_map) {
		if (id_map.find(detection_camera.particles_parent) != id_map.end()) {
			detection_camera.particles_parent = id_map[detection_camera.particles_parent];
		}

		if (id_map.find(detection_camera.camera_light) != id_map.end()) {
			detection_camera.camera_light = id_map[detection_camera.camera_light];
		}

		if (id_map.find(detection_camera.camera_model) != id_map.end()) {
			detection_camera.camera_model = id_map[detection_camera.camera_model];
		}
	}

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

	static void update_ids(World &world, Entity entity, std::unordered_map<Entity, Entity> id_map) {
		if (world.has_component<Interactable>(entity)) {
			auto &interactable = world.get_component<Interactable>(entity);
			update_interactable_ids(interactable, std::move(id_map));
		}

		if (world.has_component<EnemyPath>(entity)) {
			auto &enemy_path = world.get_component<EnemyPath>(entity);
			update_enemy_path_ids(enemy_path, std::move(id_map));
		}

		if (world.has_component<AgentData>(entity)) {
			auto &agent_data = world.get_component<AgentData>(entity);
			update_agent_data_ids(agent_data, std::move(id_map));
		}

		if (world.has_component<HackerData>(entity)) {
			auto &hacker_data = world.get_component<HackerData>(entity);
			update_hacker_data_ids(hacker_data, std::move(id_map));
		}

		if (world.has_component<DetectionCamera>(entity)) {
			auto &detection_camera = world.get_component<DetectionCamera>(entity);
			update_detection_camera_ids(detection_camera, std::move(id_map));
		}
	}
};

#endif //SILENCE_COMPONENT_VISITOR_H
