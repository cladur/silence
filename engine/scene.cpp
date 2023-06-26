#include "scene.h"
#include "animation/ecs/animation_instance.h"
#include "components/exploding_box_component.h"
#include "components/wall_cube_component.h"
#include "display/display_manager.h"
#include "ecs/systems/detection_camera_system.h"
#include "ecs/systems/dialogue_system.h"
#include "ecs/systems/hacker_movement_system.h"
#include "ecs/systems/interactable_system.h"
#include "ecs/systems/light_switcher_system.h"
#include "ecs/systems/platform_system.h"
#include "ecs/systems/rotator_system.h"
#include "ecs/systems/wall_cube_system.h"
#include "components/main_menu_component.h"
#include "ecs/world.h"
#include "editor/editor.h"
#include "managers/animation/ecs/animation_instance.h"
#include "physics/physics_manager.h"
#include "render/ecs/model_instance.h"
#include "render/ecs/skinned_model_instance.h"
#include "render/render_manager.h"

#include "animation/ecs/animation_system.h"
#include "animation/ecs/attachment_system.h"
#include "audio/audio_manager.h"
#include "audio/ecs/fmod_emitter_system.h"
#include "components/checkpoint_component.h"
#include "components/dialogue_trigger_component.h"
#include "components/fmod_emitter_component.h"
#include "components/taggable_component.h"
#include "ecs/systems/agent_movement_system.h"
#include "ecs/systems/agent_system.h"
#include "ecs/systems/cable_system.h"
#include "ecs/systems/checkpoint_collider_draw.h"
#include "ecs/systems/checkpoint_system.h"
#include "ecs/systems/collider_draw.h"
#include "ecs/systems/dialogue_collider_draw.h"
#include "ecs/systems/enemy_path_draw_system.h"
#include "ecs/systems/main_menu_system.h"
#include "ecs/systems/enemy_pathing.h"
#include "ecs/systems/enemy_system.h"
#include "ecs/systems/hacker_system.h"
#include "ecs/systems/highlight_system.h"
#include "ecs/systems/isolated_entities_system.h"
#include "ecs/systems/root_parent_system.h"
#include "ecs/systems/taggable_system.h"
#include "gameplay/gameplay_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "managers/physics/ecs/physics_system.h"
#include "render/ecs/billboard_component.h"
#include "render/ecs/billboard_system.h"
#include "render/ecs/decal_system.h"
#include "render/ecs/frustum_draw_system.h"
#include "render/ecs/light_render_system.h"
#include "render/ecs/particle_render_system.h"
#include "render/ecs/render_system.h"
#include "render/ecs/skinned_render_system.h"


#define COLLISION_TEST_ENTITY 4

Scene::Scene() {
	ZoneNamedNC(Zone1, "Scene::Scene()", tracy::Color::RebeccaPurple, true);
	// ECS
	world.startup();
	world.parent_scene = this;

	ZoneNamedNC(Zone2, "Scene::Scene()::Components", tracy::Color::Green, true);
	{
		world.register_component<Name>();
		world.register_component<Transform>();
		world.register_component<RigidBody>();
		world.register_component<Parent>();
		world.register_component<Children>();
		world.register_component<ModelInstance>();
		world.register_component<SkinnedModelInstance>();
		world.register_component<AnimationInstance>();
		world.register_component<FmodListener>();
		world.register_component<Camera>();
		world.register_component<StaticTag>();
		world.register_component<ColliderTag>();
		world.register_component<ColliderSphere>();
		world.register_component<ColliderAABB>();
		world.register_component<ColliderCapsule>();
		world.register_component<ColliderOBB>();
		world.register_component<Light>();
		world.register_component<AgentData>();
		world.register_component<HackerData>();
		world.register_component<EnemyPath>();
		world.register_component<Interactable>();
		world.register_component<Attachment>();
		world.register_component<Platform>();
		world.register_component<ExplodingBox>();
		world.register_component<EnemyData>();
		world.register_component<Billboard>();
		world.register_component<PathNode>();
		world.register_component<PathParent>();
		world.register_component<Taggable>();
		world.register_component<FMODEmitter>();
		world.register_component<Highlight>();
		world.register_component<ParticleEmitter>();
		world.register_component<DetectionCamera>();
		world.register_component<CableParent>();
		world.register_component<Rotator>();
		world.register_component<LightSwitcher>();
		world.register_component<Decal>();
		world.register_component<WallCube>();
		world.register_component<DialogueTrigger>();
		world.register_component<Checkpoint>();
		world.register_component<MainMenu>();
	}
	// Components
	{ register_main_systems(); }

	auto &physics_manager = PhysicsManager::get();
	physics_manager.add_collision_layer("default");
	physics_manager.add_collision_layer("hacker");
	physics_manager.add_collision_layer("agent");
	physics_manager.add_collision_layer("camera");
	physics_manager.add_collision_layer("obstacle");
	physics_manager.add_collision_layer("taggable");

	physics_manager.set_layers_no_collision("default", "hacker");
	physics_manager.set_layers_no_collision("agent", "hacker");
	physics_manager.set_layers_no_collision("camera", "hacker");

	physics_manager.set_layers_no_collision("camera", "default");
	physics_manager.set_layers_no_collision("camera", "agent");
	physics_manager.set_layers_no_collision("camera", "taggable");
	physics_manager.set_layers_no_collision("camera", "obstacle");
}

void Scene::register_main_systems() {
	// Systems
	world.register_system<AnimationSystem>(UpdateOrder::PrePreAnimation);

	// Render stuff
	world.register_system<RenderSystem>(UpdateOrder::PostPhysics);
	world.register_system<SkinnedRenderSystem>(UpdateOrder::PostPhysics);
	world.register_system<CheckpointColliderDrawSystem>(UpdateOrder::PostPhysics);
	world.register_system<DialogueColliderDrawSystem>(UpdateOrder::PostPhysics);
	world.register_system<ColliderDrawSystem>(UpdateOrder::PostPhysics);
	world.register_system<FrustumDrawSystem>(UpdateOrder::PostPhysics);
	world.register_system<LightRenderSystem>(UpdateOrder::PostPhysics);
	world.register_system<EnemyPathDraw>(UpdateOrder::PostPhysics);
	world.register_system<BillboardSystem>(UpdateOrder::PostPhysics);
	world.register_system<ParticleRenderSystem>(UpdateOrder::PostPhysics);
	world.register_system<DecalSystem>(UpdateOrder::PostPhysics);
	world.register_system<WallCubeSystem>(UpdateOrder::PostPhysics);

	// Transform
	world.register_system<IsolatedEntitiesSystem>(UpdateOrder::DuringPhysics);
	world.register_system<RootParentSystem>(UpdateOrder::DuringPhysics);
	world.register_system<AttachmentSystem>(UpdateOrder::PostAnimation);
	world.register_system<CableSystem>(UpdateOrder::PreAnimation);
}

void Scene::register_game_systems() {
	ZoneScopedN("Scene::register_game_systems");
	// Physics
	world.register_system<PhysicsSystem>();

	// Agents
	auto agent_system = world.register_system<AgentSystem>();
	world.register_system<AgentMovementSystem>(UpdateOrder::DuringPhysics);
	auto hacker_system = world.register_system<HackerSystem>();
	world.register_system<HackerMovementSystem>(UpdateOrder::DuringPhysics);
	world.register_system<CollisionSystem>(UpdateOrder::DuringPhysics);
	world.register_system<EnemySystem>(UpdateOrder::PostAnimation);
	world.register_system<TaggableSystem>();
	//world.register_system<EnemyPathing>();
	world.register_system<InteractableSystem>();
	world.register_system<PlatformSystem>();
	world.register_system<FMODEmitterSystem>(UpdateOrder::PrePreAnimation);
	world.register_system<HighlightSystem>(UpdateOrder::PrePreAnimation);
	world.register_system<DetectionCameraSystem>();

	world.register_system<LightSwitcherSystem>();
	world.register_system<RotatorSystem>();
	world.register_system<DialogueSystem>();
	world.register_system<CheckpointSystem>();
	world.register_system<MainMenuSystem>(UpdateOrder::PrePreAnimation);

	GameplayManager::get().set_agent_system(agent_system);
	GameplayManager::get().set_hacker_system(hacker_system);
}

void Scene::update(float dt) {
	ZoneScopedN("Scene::update");

	for (Entity entity : entities) {
		if (world.has_component<Camera>(entity) && world.has_component<Transform>(entity)) {
			auto &camera = world.get_component<Camera>(entity);
			auto &transform = world.get_component<Transform>(entity);
			if (camera.right_side) {
				get_render_scene().right_camera_params = camera;
				get_render_scene().right_camera_transform = transform;
			} else {
				get_render_scene().left_camera_params = camera;
				get_render_scene().left_camera_transform = transform;
			}
		}
	}

	GameplayManager::get().update(world, dt);
}

RenderScene &Scene::get_render_scene() const {
	RenderManager &render_manager = RenderManager::get();
	return render_manager.render_scenes[render_scene_idx];
}

void Scene::save_to_file(const std::string &path) {
	if (!path.empty()) {
		this->path = path;
	}
	if (this->path.empty()) {
		return;
	}
	auto json = SceneManager::save_scene(world, entities);
	SceneManager::save_json_to_file(this->path, json);
}

void Scene::load_from_file(const std::string &path) {
	ZoneScopedN("Scene::load_from_file");
	if (path.empty()) {
		SPDLOG_ERROR("Wrong path: ", path, " should not be empty");
		assert(false);
	}
	this->path = path;
	if (this->path.empty()) {
		return;
	}
	std::ifstream file(this->path);
	if (!file.is_open()) {
		SPDLOG_WARN("Failed to open file: ", this->path);
		return;
	}
	nlohmann::json scene_json = nlohmann::json::parse(file);
	file.close();
	SceneManager::load_scene_from_json_file(world, scene_json, "", entities);
}