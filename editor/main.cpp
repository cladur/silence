#include "audio/audio_manager.h"
#include "display/display_manager.h"
#include "font/font_manager.h"
#include "input/input_manager.h"
#include "managers/render/common/material.h"
#include <string>

#include "imgui_impl_opengl3.h"
#include "managers/render/ecs/render_system.h"
#include "render/render_manager.h"

#include "components/children_component.h"
#include "components/collider_aabb.h"
#include "components/collider_obb.h"
#include "components/collider_sphere.h"
#include "components/gravity_component.h"
#include "components/name_component.h"
#include "components/parent_component.h"
#include "components/rigidbody_component.h"
#include "components/transform_component.h"

#include "ecs/ecs_manager.h"
#include "ecs/systems/collider_components_factory.h"
#include "ecs/systems/collision_system.h"
#include "ecs/systems/parent_system.h"
#include "ecs/systems/physics_system.h"

#include "audio/fmod_listener_system.h"
#include "components/fmod_listener_component.h"
#include "imgui_impl_glfw.h"

#include "scene/scene_manager.h"
#include "serialization.h"
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <fstream>

#include "core/camera/camera.h"

#include <ImGuizmo.h>
#include <nfd.h>
#include <glm/gtc/type_ptr.hpp>

ECSManager ecs_manager;
AudioManager audio_manager;
InputManager input_manager;

Camera camera(glm::vec3(0.0f, 0.0f, -25.0f));

bool controlling_camera = false;
bool viewport_hovered = false;
std::vector<Entity> entities_selected;
Entity last_entity_selected = 0;

ImGuizmo::OPERATION current_gizmo_operation(ImGuizmo::TRANSLATE);
ImGuizmo::MODE current_gizmo_mode(ImGuizmo::WORLD);

void default_mappings() {
	input_manager.add_action("move_forward");
	input_manager.add_key_to_action("move_forward", InputKey::W);
	input_manager.add_key_to_action("move_forward", InputKey::GAMEPAD_LEFT_STICK_Y_POSITIVE);
	input_manager.add_action("move_backward");
	input_manager.add_key_to_action("move_backward", InputKey::S);
	input_manager.add_key_to_action("move_backward", InputKey::GAMEPAD_LEFT_STICK_Y_NEGATIVE);
	input_manager.add_action("move_left");
	input_manager.add_key_to_action("move_left", InputKey::A);
	input_manager.add_key_to_action("move_left", InputKey::GAMEPAD_LEFT_STICK_X_NEGATIVE);
	input_manager.add_action("move_right");
	input_manager.add_key_to_action("move_right", InputKey::D);
	input_manager.add_key_to_action("move_right", InputKey::GAMEPAD_LEFT_STICK_X_POSITIVE);
	input_manager.add_action("move_up");
	input_manager.add_key_to_action("move_up", InputKey::E);
	input_manager.add_key_to_action("move_up", InputKey::GAMEPAD_BUTTON_A);
	input_manager.add_action("move_down");
	input_manager.add_key_to_action("move_down", InputKey::Q);
	input_manager.add_key_to_action("move_down", InputKey::GAMEPAD_BUTTON_B);
	input_manager.add_action("move_faster");
	input_manager.add_key_to_action("move_faster", InputKey::LEFT_SHIFT);

	input_manager.add_action("control_camera");
	input_manager.add_key_to_action("control_camera", InputKey::MOUSE_RIGHT);

	input_manager.add_action("select_multiple");
	input_manager.add_key_to_action("select_multiple", InputKey::LEFT_CONTROL);
	input_manager.add_action("select_rows");
	input_manager.add_key_to_action("select_rows", InputKey::LEFT_SHIFT);
	input_manager.add_action("select_mode");
	input_manager.add_key_to_action("select_mode", InputKey::Q);

	input_manager.add_action("translate_mode");
	input_manager.add_key_to_action("translate_mode", InputKey::W);
	input_manager.add_action("rotate_mode");
	input_manager.add_key_to_action("rotate_mode", InputKey::E);
	input_manager.add_action("scale_mode");
	input_manager.add_key_to_action("scale_mode", InputKey::R);
	input_manager.add_action("toggle_gizmo_mode");
	input_manager.add_key_to_action("toggle_gizmo_mode", InputKey::T);

	input_manager.add_action("delete");
	input_manager.add_key_to_action("delete", InputKey::BACKSPACE);
}

void default_ecs_manager_init() {
	ecs_manager.startup();

	ecs_manager.register_component<Transform>();
	ecs_manager.register_component<RigidBody>();
	ecs_manager.register_component<Gravity>();
	ecs_manager.register_component<Parent>();
	ecs_manager.register_component<Children>();
	ecs_manager.register_component<ModelInstance>();
	ecs_manager.register_component<FmodListener>();
	ecs_manager.register_component<ColliderTag>();
	ecs_manager.register_component<ColliderSphere>();
	ecs_manager.register_component<ColliderAABB>();
	ecs_manager.register_component<ColliderOBB>();
	ecs_manager.register_component<Name>();
}

void demo_entities_init(std::vector<Entity> &entities) {
	std::default_random_engine random_generator; // NOLINT(cert-msc51-cpp)
	std::uniform_real_distribution<float> rand_position(-20.0f, 20.0f);
	std::uniform_real_distribution<float> rand_rotation(0.0f, 0.0f);
	std::uniform_real_distribution<float> rand_scale(3.0f, 5.0f);
	std::uniform_real_distribution<float> rand_gravity(-9.8f, -9.8f);

	float scale = rand_scale(random_generator);

	for (unsigned int &entity : entities) {
		entity = ecs_manager.create_entity();

		ecs_manager.add_component<Name>(entity, Name("Entity " + std::to_string(entity)));

		ecs_manager.add_component<Gravity>(entity, { glm::vec3(0.0f, rand_gravity(random_generator), 0.0f) });

		ecs_manager.add_component(entity,
				RigidBody{ .velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f) });

		Transform transform = Transform{ glm::vec3(rand_position(random_generator),
												 rand_position(random_generator) + 20.0f,
												 rand_position(random_generator)),
			glm::vec3(rand_rotation(random_generator), -50.0f, rand_rotation(random_generator)), glm::vec3(1.0f) };
		ecs_manager.add_component(entity, transform);

		ColliderComponentsFactory::add_collider_component(
				entity, ColliderAABB{ transform.get_position(), transform.get_scale(), true });

		//		Handle<ModelInstance> hndl = RenderManager::get()->add_instance("electricBox/electricBox.pfb");
		//		Handle<ModelInstance> hndl = RenderManager::get()->add_instance("Agent/agent_idle.pfb");
		ecs_manager.add_component<ModelInstance>(entity, ModelInstance("woodenBox/woodenBox.pfb"));
	}

	auto listener = ecs_manager.create_entity();
	ecs_manager.add_component(listener, Transform{ glm::vec3(0.0f, 0.0f, -25.0f), glm::vec3(0.0f), glm::vec3(1.0f) });
	// Later on attach FmodListener component to camera
	ecs_manager.add_component<FmodListener>(listener,
			FmodListener{ .listener_id = SILENCE_FMOD_LISTENER_DEBUG_CAMERA,
					.prev_frame_position = glm::vec3(0.0f, 0.0f, -25.0f) });
	entities.push_back(listener);

	Entity floor = ecs_manager.create_entity();

	Transform transform = Transform{ glm::vec3(0.0f, -20.0f, 0.0f), glm::vec3(0.0f), glm::vec3(20.0f, 1.0f, 20.0f) };
	ecs_manager.add_component(floor, transform);

	ColliderComponentsFactory::add_collider_component(
			floor, ColliderAABB{ transform.get_position(), transform.get_scale(), false });

	entities.push_back(floor);
}

void demo_collision_init(Entity &entity) {
	entity = ecs_manager.create_entity();

	Transform transform = Transform{ glm::vec3(0.0f), glm::vec3(45.0f), glm::vec3(1.0f) };
	ecs_manager.add_component(entity, transform);

	ColliderOBB c{};
	c.center = transform.get_position();
	c.range = transform.get_scale();
	c.set_orientation(transform.get_euler_rot());
	c.is_movable = true;
	ColliderComponentsFactory::add_collider_component(entity, c);

	// ecs_manager.add_component<MeshInstance>(
	// 		entity, { render_manager.get_mesh("box"), render_manager.get_material("default_mesh") });
}

void demo_collision_sphere(std::vector<Entity> &entities) {
	std::default_random_engine random_generator; // NOLINT(cert-msc51-cpp)
	std::uniform_real_distribution<float> rand_position(-20.0f, 20.0f);
	std::uniform_real_distribution<float> rand_gravity(-40.0f, -20.0f);

	for (Entity &entity : entities) {
		entity = ecs_manager.create_entity();

		Transform transform = Transform{ glm::vec3(rand_position(random_generator), rand_position(random_generator),
												 rand_position(random_generator)),
			glm::vec3(0.0f), glm::vec3(1.0f) };
		ecs_manager.add_component(entity, transform);

		ColliderComponentsFactory::add_collider_component(
				entity, ColliderSphere{ transform.get_position(), transform.get_scale().x, true });

		ecs_manager.add_component<Gravity>(entity, { glm::vec3(0.0f, rand_gravity(random_generator), 0.0f) });

		ecs_manager.add_component(entity,
				RigidBody{ .velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f) });
	}
}

void demo_collision_obb(std::vector<Entity> &entities) {
	std::default_random_engine random_generator; // NOLINT(cert-msc51-cpp)
	std::uniform_real_distribution<float> rand_position(-10.0f, 10.0f);
	std::uniform_real_distribution<float> rand_rotation(-90.0f, 90.0f);
	std::uniform_real_distribution<float> rand_gravity(-40.0f, -20.0f);

	for (Entity &entity : entities) {
		entity = ecs_manager.create_entity();

		Transform transform = Transform{ glm::vec3(rand_position(random_generator), rand_position(random_generator),
												 rand_position(random_generator)),
			glm::vec3(rand_rotation(random_generator)), glm::vec3(1.0f) };
		ecs_manager.add_component(entity, transform);

		ColliderOBB c{};
		c.center = transform.get_position();
		c.range = transform.get_scale();
		c.set_orientation(transform.get_euler_rot());
		c.is_movable = true;

		ColliderComponentsFactory::add_collider_component(entity, c);

		ecs_manager.add_component<Gravity>(entity, { glm::vec3(0.0f, rand_gravity(random_generator), 0.0f) });

		ecs_manager.add_component(entity,
				RigidBody{ .velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f) });
	}
}

bool display_manager_init() {
	auto display_manager_result = DisplayManager::get()->startup(true);
	if (display_manager_result == DisplayManager::Status::Ok) {
		SPDLOG_INFO("Initialized display manager");
	} else {
		SPDLOG_ERROR("Failed to initialize the display manager. Status: ({}) {}",
				magic_enum::enum_integer(display_manager_result), magic_enum::enum_name(display_manager_result));
		return false;
	}

	return true;
}

void handle_camera(Camera &cam, float dt) {
	float forward = input_manager.get_axis("move_backward", "move_forward");
	float right = input_manager.get_axis("move_left", "move_right");
	float up = input_manager.get_axis("move_down", "move_up");

	if (input_manager.is_action_pressed("move_faster")) {
		dt *= 3.0f;
	}

	cam.move_forward(forward * dt);
	cam.move_right(right * dt);
	cam.move_up(up * dt);

	glm::vec2 mouse_delta = input_manager.get_mouse_delta();
	cam.rotate(mouse_delta.x * dt, mouse_delta.y * dt);
}

int main() {
	SPDLOG_INFO("Starting up engine systems...");

	display_manager_init();
	input_manager.startup();
	RenderManager::get()->startup();

	// ECS ----------------------------------------

	default_ecs_manager_init();
	auto physics_system = ecs_manager.register_system<PhysicsSystem>();
	auto collision_system = ecs_manager.register_system<CollisionSystem>();
	auto parent_system = ecs_manager.register_system<ParentSystem>();
	auto fmod_listener_system = ecs_manager.register_system<FmodListenerSystem>();

	auto render_system = ecs_manager.register_system<RenderSystem>();
	render_system->startup();

	physics_system->startup();
	collision_system->startup();
	parent_system->startup();
	fmod_listener_system->startup();
	FontManager::get()->startup();

	NFD_Init();

	std::vector<Entity> entities(50);
	demo_entities_init(entities);

	std::vector<Entity> spheres(10);
	demo_collision_sphere(spheres);

	std::vector<Entity> obbs(10);
	demo_collision_obb(obbs);

	Entity collision_tester;
	demo_collision_init(collision_tester);

	audio_manager.startup();
	audio_manager.load_bank("Music");
	audio_manager.load_bank("SFX");
	audio_manager.load_bank("Ambience");
	audio_manager.load_sample_data();

	FontManager::get()->load_font("resources/fonts/PoltawskiNowy.ttf", 48);

	//Map inputs
	default_mappings();

	// Run the game.
	bool show_cvar_editor = false;
	bool show_ecs_logs = false;
	bool show_demo_window = false;
	bool physics_system_enabled = false;
	int max_entities = 100;
	int imgui_entities_count = 50;
	int frames_count = 0;

	float target_frame_time = 1.0f / (float)DisplayManager::get()->get_refresh_rate();
	float dt = target_frame_time;

	// TEST FOR 3D AUDIO
	glm::vec3 sound_position = glm::vec3(0.0f, 0.0f, 0.0f);
	EventReference test_pluck = EventReference("test_pluck");
	// #################

	bool should_run = true;
	nlohmann::json scene;
	while (should_run) {
		// GAME LOGIC
		auto start_time = std::chrono::high_resolution_clock::now();

		//imgui new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		DisplayManager::get()->poll_events();

		if (!controlling_camera) {
			if (input_manager.is_action_just_pressed("translate_mode")) {
				current_gizmo_operation = ImGuizmo::TRANSLATE;
			} else if (input_manager.is_action_just_pressed("rotate_mode")) {
				current_gizmo_operation = ImGuizmo::ROTATE;
			} else if (input_manager.is_action_just_pressed("scale_mode")) {
				current_gizmo_operation = ImGuizmo::SCALE;
			}

			if (input_manager.is_action_just_pressed("toggle_gizmo_mode")) {
				if (current_gizmo_mode == ImGuizmo::WORLD) {
					current_gizmo_mode = ImGuizmo::LOCAL;
				} else {
					current_gizmo_mode = ImGuizmo::WORLD;
				}
			}
		}

		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save")) {
				SPDLOG_INFO("Saving scene...");
			}
			if (ImGui::MenuItem("Save as...")) {
				SPDLOG_INFO("Saving scene as...");
			}
			if (ImGui::MenuItem("Load")) {
				SPDLOG_INFO("Loading scene...");
				nfdchar_t *out_path;
				nfdfilteritem_t filter_item[2] = { { "Source code", "c,cpp,cc" }, { "Headers", "h,hpp" } };
				nfdresult_t result = NFD_OpenDialog(&out_path, filter_item, 2, nullptr);
				if (result == NFD_OKAY) {
					puts("Success!");
					puts(out_path);
					NFD_FreePath(out_path);
				} else if (result == NFD_CANCEL) {
					puts("User pressed cancel.");
				} else {
					printf("Error: %s\n", NFD_GetError());
				}
			}
			if (ImGui::MenuItem("Exit")) {
				should_run = false;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();

		ImGui::Begin("Viewport");

		viewport_hovered = ImGui::IsWindowHovered();

		if (ImGui::Button("Select")) {
		}
		ImGui::SameLine();
		if (ImGui::Button("Move")) {
			current_gizmo_operation = ImGuizmo::TRANSLATE;
		}
		ImGui::SameLine();
		if (ImGui::Button("Rotate")) {
			current_gizmo_operation = ImGuizmo::ROTATE;
		}
		ImGui::SameLine();
		if (ImGui::Button("Scale")) {
			current_gizmo_operation = ImGuizmo::SCALE;
		}
		ImGui::SameLine();
		if (current_gizmo_mode == ImGuizmo::WORLD) {
			if (ImGui::Button("World")) {
				current_gizmo_mode = ImGuizmo::LOCAL;
			}
		} else {
			if (ImGui::Button("Local")) {
				current_gizmo_mode = ImGuizmo::WORLD;
			}
		}

		// Get viewport size
		static ImVec2 last_viewport_size = ImVec2(0, 0);
		ImVec2 viewport_size = ImGui::GetContentRegionAvail();
		if (viewport_size.x != last_viewport_size.x || viewport_size.y != last_viewport_size.y) {
			// Resize the framebuffer
			RenderManager::get()->resize_framebuffer(viewport_size.x, viewport_size.y);
			last_viewport_size = viewport_size;
		}

		uint32_t render_image = RenderManager::get()->render_framebuffer.get_texture_id();
		ImGui::Image((void *)(intptr_t)render_image, viewport_size, ImVec2(0, 1), ImVec2(1, 0));

		// Draw gizmo
		ImGuiIO &io = ImGui::GetIO();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewport_size.x, viewport_size.y);
		ImGuizmo::SetDrawlist();

		RenderManager *render_manager = RenderManager::get();

		glm::mat4 *view = &render_manager->view;
		glm::mat4 *projection = &render_manager->projection;

		if (!entities_selected.empty()) {
			// If we have entities selected, we want to take their average position and use that as the pivot point
			// for the gizmo

			glm::mat4 temp_matrix = glm::mat4(1.0f);

			if (entities_selected.size() > 1) {
				auto average_position = glm::vec3(0.0f);
				for (auto &entity : entities_selected) {
					if (ecs_manager.has_component<Transform>(entity)) {
						auto &transform = ecs_manager.get_component<Transform>(entity);
						average_position += transform.get_position();
					}
				}

				temp_matrix = glm::translate(glm::mat4(1.0f), average_position / (float)entities_selected.size());
			} else {
				auto &transform = ecs_manager.get_component<Transform>(entities_selected[0]);
				temp_matrix = transform.get_global_model_matrix();
			}

			if (ImGuizmo::Manipulate(glm::value_ptr(*view), glm::value_ptr(*projection), current_gizmo_operation,
						current_gizmo_mode, glm::value_ptr(temp_matrix), nullptr, nullptr)) {
				// Gizmo handles our final world transform, but we need to update our local transform (pos, orient,
				// scale) In order to do that, we extract the local transform from the world transform, by
				// multiplying by the inverse of the parent's world transform From there, we can decompose the local
				// transform into its components (pos, orient, scale)
				glm::vec3 skew;
				glm::vec4 perspective;
				// if (last_selected->parent) {
				// 	glm::decompose(glm::inverse(last_selected->parent->transform.modelMatrix) * tempMatrix,
				// 			last_selected->transform.scale, last_selected->transform.orient,
				// 			last_selected->transform.pos, skew, perspective);
				// } else {
				// 	glm::decompose(temp_matrix, last_selected->transform.scale, last_selected->transform.orient,
				// 			last_selected->transform.pos, skew, perspective);
				// }

				// last_selected->transform.isDirty = true;
			}
		}

		ImGui::End();

		ImGui::Begin("Scene");

		for (auto &entity : entities) {
			std::string entity_name = std::to_string(entity);
			if (ecs_manager.has_component<Name>(entity)) {
				entity_name = ecs_manager.get_component<Name>(entity).name;
			}

			static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
					ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

			ImGuiTreeNodeFlags node_flags = base_flags;

			const bool is_selected =
					(std::find(entities_selected.begin(), entities_selected.end(), entity) != entities_selected.end());

			if (is_selected) {
				node_flags |= ImGuiTreeNodeFlags_Selected;
			}

			bool node_open = ImGui::TreeNodeEx(entity_name.c_str(), node_flags);

			if (ImGui::IsItemClicked()) {
				if (input_manager.is_action_pressed("select_multiple")) {
					if (is_selected) {
						entities_selected.erase(std::remove(entities_selected.begin(), entities_selected.end(), entity),
								entities_selected.end());
					} else {
						entities_selected.push_back(entity);
					}
				} else if (input_manager.is_action_pressed("select_rows") && last_entity_selected != 0) {
					// Select all entities between last_entity_selected and entity
					entities_selected.clear();
					uint32_t min = std::min(last_entity_selected, entity);
					uint32_t max = std::max(last_entity_selected, entity);
					for (uint32_t i = min; i <= max; i++) {
						entities_selected.push_back(i);
					}
				} else {
					entities_selected.clear();
					entities_selected.push_back(entity);
				}
				last_entity_selected = entity;
			}

			if (node_open) {
				ImGui::Text("TODO: Show entity children?");

				ImGui::TreePop();
			}
		}

		ImGui::End();

		ImGui::Begin("Inspector");

		if (!entities_selected.empty()) {
			Entity active_entity = entities_selected.back();
			// List all components of entity
			if (ecs_manager.has_component<Name>(active_entity)) {
				Name &name = ecs_manager.get_component<Name>(active_entity);
				ImGui::Text("%s", name.name.c_str());
			} else {
				ImGui::Text("Entity %d", active_entity);
			}
			if (ecs_manager.has_component<Transform>(active_entity)) {
				if (ImGui::TreeNode("Transform")) {
					auto &transform = ecs_manager.get_component<Transform>(active_entity);
					glm::vec3 pos = transform.get_position();
					glm::vec3 rot = transform.get_euler_rot();
					glm::vec3 scale = transform.get_scale();
					ImGui::Text("Pos: %.3f, %.3f, %.3f", pos.x, pos.y, pos.z);
					ImGui::Text("Rot: %.3f, %.3f, %.3f", rot.x, rot.y, rot.z);
					ImGui::Text("Scale: %.3f, %.3f, %.3f", scale.x, scale.y, scale.z);

					ImGui::TreePop();
				}
			}
			if (ecs_manager.has_component<ModelInstance>(active_entity)) {
				if (ImGui::TreeNode("ModelInstance")) {
					auto &model_instance = ecs_manager.get_component<ModelInstance>(active_entity);
					ImGui::Text("Model: %s", render_manager->get_model(model_instance.model_handle).name.c_str());
					ImGui::Text("Material: %s", magic_enum::enum_name(model_instance.material_type).data());

					ImGui::TreePop();
				}
			}
		} else {
			ImGui::Text("No entity selected");
		}

		ImGui::End();

		ImGui::Begin("Resources");

		ImGui::Text("TODO: List resources");

		ImGui::End();

		if ((viewport_hovered && input_manager.is_action_pressed("control_camera") || controlling_camera)) {
			controlling_camera = true;
			handle_camera(camera, dt);
			DisplayManager::get()->capture_mouse(true);
		}

		if (input_manager.is_action_just_released("control_camera")) {
			controlling_camera = false;
			DisplayManager::get()->capture_mouse(false);
		}

		ImGui::Begin("Settings");

		ImGui::Checkbox("Show console ecs logs", &show_ecs_logs);

		ImGui::Checkbox("Show demo window", &show_demo_window);

		ImGui::Checkbox("Show CVAR editor", &show_cvar_editor);

		ImGui::Checkbox("Physics system", &physics_system_enabled);

		if (show_demo_window) {
			ImGui::ShowDemoWindow();
		}

		ImGui::DragInt("Entities count", &imgui_entities_count, 1, 1, max_entities);

		// 3D SOUND DEMO
		ImGui::SliderFloat3("Sound position", &sound_position[0], -100.0f, 100.0f);

		if (ImGui::Button("Play pluck")) {
			audio_manager.play_one_shot_3d(test_pluck, sound_position);
		}
		// 3D SOUND DEMO

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
				ImGui::GetIO().Framerate);

		ImGui::End();

		if (show_cvar_editor) {
			CVarSystem::get()->draw_imgui_editor();
		}

		if (DisplayManager::get()->window_should_close()) {
			should_run = false;
		}

		if (physics_system_enabled) {
			physics_system->update(dt);
		}

		collision_system->update();

		parent_system->update();
		render_system->update();

		debug_draw::draw_line(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 0.0f, 0.0f));
		debug_draw::draw_line(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(10.0f, 0.0f, 0.0f));

		debug_draw::draw_line(glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(10.0f, 0.0f, 2.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		debug_draw::draw_box(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		debug_draw::draw_box(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(10.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		input_manager.process_input();

		RenderManager::get()->draw();

		fmod_listener_system->update(dt);
		audio_manager.update();

		frames_count++;
		auto stop_time = std::chrono::high_resolution_clock::now();

		// TODO: This is a hack to make sure we don't go over the target frame time.
		// We need to make calculate dt properly and make target frame time changeable.
		while (std::chrono::duration<float, std::chrono::seconds::period>(stop_time - start_time).count() <
				target_frame_time) {
			stop_time = std::chrono::high_resolution_clock::now();
		}

		FrameMark;
	}

	// Shut everything down, in reverse order.
	SPDLOG_INFO("Shutting down engine subsystems...");
	input_manager.shutdown();
	audio_manager.shutdown();
	RenderManager::get()->shutdown();
	DisplayManager::get()->shutdown();

	return 0;
}
