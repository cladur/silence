#include "ui_render_system.h"
#include "core/components/transform_component.h"
#include "core/components/ui_text_component.h"
#include "ecs/ecs_manager.h"
#include <render/render_system.h>
#include <render/ui/text/font_manager.h>

extern ECSManager ecs_manager;
extern FontManager font_manager;

void UiRenderSystem::startup() {
	Signature signature;
	signature.set(ecs_manager.get_component_type<Transform>());
	signature.set(ecs_manager.get_component_type<MeshInstance>());
	signature.set(ecs_manager.get_component_type<UIText>());
	ecs_manager.set_system_component_whitelist<UiRenderSystem>(signature);
}

void UiRenderSystem::update(RenderManager &render_manager) {
	for (auto const &entity : entities) {
		auto &transform = ecs_manager.get_component<Transform>(entity);
		auto &mesh_instance = ecs_manager.get_component<MeshInstance>(entity);
		auto &ui_text = ecs_manager.get_component<UIText>(entity);

		glm::vec3 current_position = glm::vec3(0.0f, 0.0f, 0.0f);

		Font &font = font_manager.get_font(ui_text.font_id);
		render_manager.glyph_sampler = font.sampler;
		render_manager.glyphs.clear();
		for (auto c : ui_text.text) {
			// check if c is the end of the string
			CharacterGlyph *g = font.get_character_glyph(c);
			Texture t = g->texture;
			render_manager.glyphs.push_back(t);
			current_position.x += (float)g->advance;

			RenderObject render_object = {};
			render_object.mesh = mesh_instance.mesh;
			render_object.material = mesh_instance.material;
			glm::mat4 m = transform.get_global_model_matrix();
			//m = glm::scale(m, glm::vec3((float)g->size.x / (float)g->advance, (float)g->size.y / (float)g->advance, 1.0f));
			m = glm::translate(m, current_position);
			render_object.transform_matrix = m;

			render_manager.renderables.push_back(render_object);
		}
	}
}
