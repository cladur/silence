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

		pe.particle_time += dt;
		// for now let's assume no more than two particles per frame.
		if (pe.particle_time > 1.0f / pe.rate) {
			ParticleData particle = ParticleData(pe, transform.get_global_position());
			pm.emit(entity, particle);
			pe.particle_time = 0.0f;
		}

	}
}