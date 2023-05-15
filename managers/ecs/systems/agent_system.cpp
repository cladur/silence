#include "agent_system.h"
#include "components/agent_data_component.h"
#include "components/light_component.h"
#include "components/transform_component.h"
#include "ecs/world.h"
#include "engine/scene.h"

#include "ecs/systems/collision_system.h"
#include "physics/physics_manager.h"

#include "input/input_manager.h"

AutoCVarFloat cvar_agent_speed("agent.speed", "agent speed", 4.0f, CVarFlags::EditCheckbox);
AutoCVarFloat cvar_camera_sensitivity(
		"settings.camera_sensitivity", "camera sensitivity", 0.1f, CVarFlags::EditCheckbox);

void AgentSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<AgentData>());

	world.set_system_component_whitelist<AgentSystem>(whitelist);
}

void AgentSystem::update(World &world, float dt) {
	InputManager &input_manager = InputManager::get();
	for (const Entity entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &agent_data = world.get_component<AgentData>(entity);
		auto &camera_pivot_tf = world.get_component<Transform>(agent_data.camera_pivot);
		auto &model_tf = world.get_component<Transform>(agent_data.model);
		auto &camera_tf = world.get_component<Transform>(agent_data.camera);

		auto camera_forward = camera_pivot_tf.get_global_forward();
		camera_forward.y = 0.0f;
		camera_forward = glm::normalize(camera_forward);
		auto camera_right = camera_pivot_tf.get_global_right();

		if (input_manager.is_action_pressed("move_forward")) {
			transform.position += camera_forward * cvar_agent_speed.get() * dt;
			transform.set_changed(true);
		}
		if (input_manager.is_action_pressed("move_backward")) {
			transform.position -= camera_forward * cvar_agent_speed.get() * dt;
			transform.set_changed(true);
		}
		if (input_manager.is_action_pressed("move_left")) {
			transform.position += camera_right * cvar_agent_speed.get() * dt;
			transform.set_changed(true);
		}
		if (input_manager.is_action_pressed("move_right")) {
			transform.position -= camera_right * cvar_agent_speed.get() * dt;
			transform.set_changed(true);
		}

		glm::vec2 mouse_delta = input_manager.get_mouse_delta();
		camera_pivot_tf.add_euler_rot(glm::vec3(mouse_delta.y, 0.0f, 0.0f) * cvar_camera_sensitivity.get() * dt);
		camera_pivot_tf.add_global_euler_rot(
				glm::vec3(0.0f, -mouse_delta.x, 0.0f) * cvar_camera_sensitivity.get() * dt);

		static glm::vec3 last_position = transform.position;

		// Lerp model_tf towards movement direction
		glm::vec3 delta_position = transform.position - last_position;
		delta_position.y = 0.0f;
		if (glm::length2(delta_position) > 0.0001f) {
			glm::vec3 direction = glm::normalize(delta_position);
			glm::vec3 forward =
					glm::normalize(glm::vec3(model_tf.get_global_forward().x, 0.0f, model_tf.get_global_forward().z));
			float angle = glm::acos(glm::dot(forward, direction)) * 0.3f;
			glm::vec3 axis = glm::cross(forward, direction);
			model_tf.add_global_euler_rot(axis * angle);
		}

		Ray ray{};
		ray.origin = transform.get_global_position() + glm::vec3(0.0f, -0.01f, 0.0f);
		ray.direction = glm::vec3(0.0f, -1.0f, 0.0f);
		glm::vec3 end = ray.origin + ray.direction;
		world.get_parent_scene()->get_render_scene().debug_draw.draw_arrow(ray.origin, end);
		HitInfo info;
		if (CollisionSystem::ray_cast(world, ray, info)) {
			// If the agent is not on the ground, move him down
			// If the agent is on 40 degree slope or more, move him down
			bool is_on_too_steep_slope =
					glm::dot(info.normal, glm::vec3(0.0f, 1.0f, 0.0f)) < glm::cos(glm::radians(40.0f));

			if (is_on_too_steep_slope || info.distance > 0.1f) {
				transform.position.y -= 8.0f * dt;
				transform.set_changed(true);
			}

			// If the agent is close enough to the floor, snap him to it
			float snap_length = 0.3f;
			if (info.distance > 0.1f && info.distance < snap_length) {
				transform.position.y = info.point.y + 0.001f;
				transform.set_changed(true);
			}
		}

		last_position = transform.position;
	}
}