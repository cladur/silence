#include "fmod_listener_system.h"
#include "audio_manager.h"
#include "components/fmod_listener_component.h"
#include "components/transform_component.h"
#include "ecs/world.h"

extern AudioManager audio_manager;

void FmodListenerSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<FmodListener>());
	signature.set(world.get_component_type<Transform>());
	world.set_system_component_whitelist<FmodListenerSystem>(signature);
}

void FmodListenerSystem::update(World &world, float dt) {
	ZoneScopedNC("FmodListenerSystem::update", 0xcacaca);
	for (auto const &entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &listener = world.get_component<FmodListener>(entity);

		glm::vec3 p = transform.get_position();
		glm::vec3 v = p - listener.prev_frame_position;
		listener.prev_frame_position = p;
		// TODO: change this after its implemented in transform
		//		glm::vec3 forward = transform.get_forward();
		//		glm::vec3 up = transform.get_up();
		glm::vec3 f = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 u = glm::vec3(0.0f, 1.0f, 0.0f);
		audio_manager.set_3d_listener_attributes(listener.listener_id, p, v, f, u);
	}
}