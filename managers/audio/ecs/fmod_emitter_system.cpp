#include "fmod_emitter_system.h"
#include "ecs/world.h"
#include <audio/audio_manager.h>
#include <components/fmod_emitter_component.h>
#include "engine/scene.h"
#include "fmod_errors.h"

#define FMOD_CHECK(x)                                                                                                  \
	do {                                                                                                               \
		FMOD_RESULT result = x;                                                                                        \
		if (result != FMOD_OK) {                                                                                       \
			SPDLOG_ERROR("Audio Manager: FMOD error! {} {}", result, FMOD_ErrorString(result));                        \
			abort();                                                                                                   \
		}                                                                                                              \
	} while (false)

void FMODEmitterSystem::startup(World &world) {
	Signature whitelist;
	whitelist.set(world.get_component_type<FMODEmitter>());
	world.set_system_component_whitelist<FMODEmitterSystem>(whitelist);
}

void FMODEmitterSystem::update(World &world, float dt) {
	ZoneScopedNC("FMODEmitterSystem::update", 0xcacaca);
	auto &am = AudioManager::get();
	for (auto const &entity : entities) {
		auto &emitter = world.get_component<FMODEmitter>(entity);

		if (emitter.first_frame) {
			// create instance and set 3d attributes if is 3d
			emitter.event_instance = am.create_event_instance(emitter.event_path);
			if (emitter.is_3d) {
				auto &transform = world.get_component<Transform>(entity);
				FMOD_3D_ATTRIBUTES attributes = AudioManager::to_3d_attributes(transform);
				FMOD_CHECK(emitter.event_instance->set3DAttributes(&attributes));
			}
			FMOD_CHECK(emitter.event_instance->start());
			emitter.first_frame = false;
		}

		if (emitter.is_playing()) {
			// update 3d properties
			if (emitter.is_3d) {
				auto &transform = world.get_component<Transform>(entity);
				FMOD_3D_ATTRIBUTES attributes = AudioManager::to_3d_attributes(transform);
				world.get_parent_scene()->get_render_scene().debug_draw.draw_sphere(transform.get_global_position(), 0.1f, glm::vec3(1.0f, 0.0f, 1.0f));
				emitter.event_instance->set3DAttributes(&attributes);
			}
		}
	}
}
