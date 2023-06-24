#ifndef SILENCE_COMPONENT_VISITOR_H
#define SILENCE_COMPONENT_VISITOR_H

#include "components/enemy_path_component.h"
#include "components/wall_cube_component.h"
#include "types.h"
#include "world.h"
#include <unordered_map>
#include <utility>

class ComponentVisitor {
private:
	static Entity update_id(Entity old_entity, std::unordered_map<Entity, Entity> id_map) {
		if (id_map.find(old_entity) != id_map.end()) {
			return id_map[old_entity];
		}
		return old_entity;
	}

	static void update_interactable_ids(Interactable &interactable, const std::unordered_map<Entity, Entity> &id_map) {
		for (auto &target : interactable.interaction_targets) {
			target = update_id(target, id_map);
		}
		interactable.cable_parent = update_id(interactable.cable_parent, id_map);
		interactable.lever = update_id(interactable.lever, id_map);
	}

	static void update_enemy_path_ids(EnemyPath &enemy_path, const std::unordered_map<Entity, Entity> &id_map) {
		enemy_path.path_parent = update_id(enemy_path.path_parent, id_map);
	}

	static void update_agent_data_ids(AgentData &agent_data, const std::unordered_map<Entity, Entity> &id_map) {
		agent_data.model = update_id(agent_data.model, id_map);
		agent_data.camera = update_id(agent_data.camera, id_map);
		agent_data.camera_pivot = update_id(agent_data.camera_pivot, id_map);
		agent_data.camera_pivot_target = update_id(agent_data.camera_pivot_target, id_map);
		agent_data.spring_arm = update_id(agent_data.spring_arm, id_map);
	}

	static void update_hacker_data_ids(HackerData &hacker_data, const std::unordered_map<Entity, Entity> &id_map) {
		hacker_data.model = update_id(hacker_data.model, id_map);
		hacker_data.camera = update_id(hacker_data.camera, id_map);
		hacker_data.camera_pivot = update_id(hacker_data.camera_pivot, id_map);
		hacker_data.scorpion_camera_transform = update_id(hacker_data.scorpion_camera_transform, id_map);
	}

	static void update_detection_camera_ids(
			DetectionCamera &detection_camera, const std::unordered_map<Entity, Entity> &id_map) {
		detection_camera.particles_parent = update_id(detection_camera.particles_parent, id_map);
		detection_camera.camera_light = update_id(detection_camera.camera_light, id_map);
		detection_camera.camera_model = update_id(detection_camera.camera_model, id_map);
	}

	static void update_wall_cube_ids(WallCube &wall_cube, const std::unordered_map<Entity, Entity> &id_map) {
		wall_cube.faces_parent = update_id(wall_cube.faces_parent, id_map);
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

	static void update_ids(World &world, Entity entity, const std::unordered_map<Entity, Entity> &id_map) {
		if (world.has_component<Interactable>(entity)) {
			auto &interactable = world.get_component<Interactable>(entity);
			update_interactable_ids(interactable, id_map);
		}

		if (world.has_component<EnemyPath>(entity)) {
			auto &enemy_path = world.get_component<EnemyPath>(entity);
			update_enemy_path_ids(enemy_path, id_map);
		}

		if (world.has_component<AgentData>(entity)) {
			auto &agent_data = world.get_component<AgentData>(entity);
			update_agent_data_ids(agent_data, id_map);
		}

		if (world.has_component<HackerData>(entity)) {
			auto &hacker_data = world.get_component<HackerData>(entity);
			update_hacker_data_ids(hacker_data, id_map);
		}

		if (world.has_component<DetectionCamera>(entity)) {
			auto &detection_camera = world.get_component<DetectionCamera>(entity);
			update_detection_camera_ids(detection_camera, id_map);
		}

		if (world.has_component<WallCube>(entity)) {
			auto &wall_cube = world.get_component<WallCube>(entity);
			update_wall_cube_ids(wall_cube, id_map);
		}
	}
};

#endif //SILENCE_COMPONENT_VISITOR_H
