#include "ui_text_render_system.h"
#include "core/components/transform_component.h"
#include "core/components/ui_text_component.h"
#include "ecs/ecs_manager.h"
#include <render/debug/debug_drawing.h>
#include <render/render_system.h>
#include <render/ui/text/font_manager.h>

extern ECSManager ecs_manager;
extern FontManager font_manager;

void UiRenderSystem::startup() {
	Signature signature;
	signature.set(ecs_manager.get_component_type<Transform>());
	//signature.set(ecs_manager.get_component_type<MeshInstance>());
	signature.set(ecs_manager.get_component_type<UIText>());
	ecs_manager.set_system_component_whitelist<UiRenderSystem>(signature);
}

void UiRenderSystem::update(RenderManager &render_manager) {
	static int counter = 0;
	for (auto const &entity : entities) {
		auto &transform = ecs_manager.get_component<Transform>(entity);
		//auto &mesh_instance = ecs_manager.get_component<MeshInstance>(entity);
		auto &ui_text = ecs_manager.get_component<UIText>(entity);

		glm::vec3 current_position = glm::vec3(0.0f, 0.0f, 0.0f);

		Font &font = font_manager.get_font(ui_text.font_id);
		render_manager.glyph_sampler = font.sampler;
		render_manager.glyphs.clear();
		ui_text.mesh.vertices.clear();
		ui_text.mesh.indices.clear();
		int current_index = 0;
		for (auto c : ui_text.text) {
			CharacterGlyph *g = font.get_character_glyph_data(c);
			if (c != ' ') {
				//create a plane vertex data for this character
				glm::vec2 top_left_uv = glm::vec2(g->atlas_pos.x, 0.0f);
				glm::vec2 top_right_uv = glm::vec2(g->atlas_pos.y, 0.0f);
				glm::vec2 bottom_right_uv = glm::vec2(g->atlas_pos.y, g->atlas_pos.z);
				glm::vec2 bottom_left_uv = glm::vec2(g->atlas_pos.x, g->atlas_pos.z);

				float xpos = current_position.x + g->bearing.x * ui_text.scale;
				float ypos = current_position.y - (g->size.y - g->bearing.y) * ui_text.scale;

				float w = g->size.x * ui_text.scale;
				float h = g->size.y * ui_text.scale;

				ui_text.mesh.vertices.push_back(
						Vertex {
								glm::vec3(xpos, ypos, 0.0f), // pos
								0.0f, // pad
								glm::vec3(0.0f), //normal
								0.0f, // pad
								ui_text.color, // color
								0.0f, // pad
								bottom_left_uv,
								0.0f, // pad
								0.0f, // pad
						}
				);

				ui_text.mesh.vertices.push_back(
						Vertex {
								glm::vec3(xpos, ypos + h, 0.0f), // pos
								0.0f, // pad
								glm::vec3(0.0f), //normal
								0.0f, // pad
								ui_text.color, // color
								0.0f, // pad
								top_left_uv,
								0.0f, // pad
								0.0f, // pad
						}
				);

				ui_text.mesh.vertices.push_back(
						Vertex {
								glm::vec3(xpos + w, ypos + h, 0.0f), // pos
								0.0f, // pad
								glm::vec3(0.0f), //normal
								0.0f, // pad
								ui_text.color, // color
								0.0f, // pad
								top_right_uv,
								0.0f, // pad
								0.0f, // pad
						}
				);

				ui_text.mesh.vertices.push_back(
						Vertex {
								glm::vec3(xpos + w, ypos, 0.0f), // pos
								0.0f, // pad
								glm::vec3(0.0f), //normal
								0.0f, // pad
								ui_text.color, // color
								0.0f, // pad
								bottom_right_uv,
								0.0f, // pad
								0.0f, // pad
						}
				);

				current_position.x += (float)g->advance * ui_text.scale;
				// = { 0, 1, 2, 2, 3, 0 };
				ui_text.mesh.indices.push_back(current_index + 0);
				ui_text.mesh.indices.push_back(current_index + 1);
				ui_text.mesh.indices.push_back(current_index + 2);
				ui_text.mesh.indices.push_back(current_index + 2);
				ui_text.mesh.indices.push_back(current_index + 3);
				ui_text.mesh.indices.push_back(current_index + 0);
				current_index += 4;
			} else {
				current_position.x += g->advance * ui_text.scale;
			}
		}
		render_manager.upload_mesh(ui_text.mesh);

		RenderObject render_object = {};
		render_object.mesh = &ui_text.mesh;
		render_object.material = render_manager.get_material("ui");
		glm::mat4 m = glm::mat4(1.0f);//transform.get_global_model_matrix();
		// todo: scale based on screen size instead of hard coded
		float s_v = ui_text.scale / 720.0f;
		float s_h = ui_text.scale / 1280.0f;
        glm::vec3 scale = glm::vec3(s_h, s_v, 1.0f);
		glm::vec3 pos = transform.get_position();
		pos.x = pos.x / 1280.0f;
		pos.y = pos.y / 720.0f;
		m = glm::translate(m, pos);
		pos.x -= current_position.x / (1280.0f * 2.0f);
		m = glm::scale(m, scale);

		render_object.transform_matrix = m;
		render_manager.renderables.push_back(render_object);
	}
}
