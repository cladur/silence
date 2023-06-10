#include "particle_render_system.h"
#include "core/components/transform_component.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include <render/render_manager.h>
#include <render/transparent_elements/particle_manager.h>

void ParticleRenderSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<ParticleEmitter>());
	world.set_system_component_whitelist<ParticleRenderSystem>(signature);
}

void ParticleRenderSystem::update(World &world, float dt) {
	auto &pm = ParticleManager::get();
	for (auto const &entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &pe = world.get_component<ParticleEmitter>(entity);

//		world.get_parent_scene()->get_render_scene().debug_draw.draw_line(
//				transform.get_global_position(),
//				transform.get_global_position() + transform.get_right(),
//				glm::vec3(1.0f, 0.0f, 0.0f));
//		world.get_parent_scene()->get_render_scene().debug_draw.draw_line(
//				transform.get_global_position(),
//				transform.get_global_position() + transform.get_global_right(),
//				glm::vec3(0.0f, 1.0f, 0.0f));

		if (pe.is_one_shot) {
			if (pe.oneshot_time_left > 0.0f) {
				pe.particle_time += dt;
				if (pe.particle_time > 1.0f / pe.rate) {
					auto glob_pos = transform.get_global_position();
					ParticleData particle = ParticleData(pe, transform, pe.position);
					ParticlePerEntityData entity_data = ParticlePerEntityData(
							pe,
							glob_pos + pe.position,
							transform.get_global_right(),
							transform.get_global_up());
					pm.emit(entity, particle, entity_data);
					pe.particle_time = 0.0f;
				}
				pe.oneshot_time_left -= dt;
			}
		} else {
			pe.particle_time += dt;
			// for now let's assume no more than two particles per frame.
			if (pe.particle_time > 1.0f / pe.rate) {
				auto glob_pos= transform.get_global_position();
				ParticleData particle = ParticleData(pe, transform, pe.position);
				ParticlePerEntityData entity_data = ParticlePerEntityData(
						pe,
						glob_pos + pe.position,
						transform.get_global_right(),
						transform.get_global_up());
				pm.emit(entity, particle, entity_data);
				pe.particle_time = 0.0f;
			}
		}
	}
}