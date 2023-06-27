#include "render/transparent_elements/ui_manager.h"

#include "render/transparent_elements/text/text_draw.h"
#include "render/transparent_elements/ui/ui_elements/ui_anchor.h"
#include "render/transparent_elements/ui/ui_elements/ui_button.h"
#include "render/transparent_elements/ui/ui_elements/ui_element.h"
#include "render/transparent_elements/ui/ui_elements/ui_image.h"
#include "render/transparent_elements/ui/ui_elements/ui_slider.h"
#include "render/transparent_elements/ui/ui_elements/ui_text.h"
#include <spdlog/spdlog.h>

void UIManager::startup() {
	text_draw = TextDraw();
	sprite_draw = SpriteDraw();

	ResourceManager::get().load_texture("anchor_debug.ktx2");
}

UIManager &UIManager::get() {
	static UIManager instance;
	return instance;
}

void UIManager::set_render_scene(RenderScene *render_scene) {
	this->render_scene = render_scene;
}

void UIManager::create_ui_scene(const std::string &scene_name) {
	scenes[scene_name] = UIScene();
}

void UIManager::delete_ui_scene(const std::string &scene_name) {
	scenes.erase(scene_name);
}

void UIManager::activate_ui_scene(const std::string &scene_name) {
	scenes[scene_name].is_active = true;
}

void UIManager::deactivate_ui_scene(const std::string &scene_name) {
	scenes[scene_name].is_active = false;
}

UIAnchor &UIManager::add_ui_anchor(const std::string &scene_name, const std::string &anchor_name) {
	// check if an element with given name already exists
	if (scenes[scene_name].anchors.find(anchor_name) != scenes[scene_name].anchors.end()) {
		SPDLOG_WARN("Anchor with name {} already exists in scene {}", anchor_name, scene_name);
		return scenes[scene_name].anchors[anchor_name];
	}
	scenes[scene_name].anchors[anchor_name] = UIAnchor();
	return scenes[scene_name].anchors[anchor_name];
}

UIImage &UIManager::add_ui_image(const std::string &scene_name, const std::string &image_name) {
	// check if an element with given name already exists
	if (scenes[scene_name].images.find(image_name) != scenes[scene_name].images.end()) {
		SPDLOG_WARN("Image with name {} already exists in scene {}", image_name, scene_name);
		return scenes[scene_name].images[image_name];
	}
	scenes[scene_name].images[image_name] = UIImage();
	return scenes[scene_name].images[image_name];
}

UIButton &UIManager::add_ui_button(const std::string &scene_name, const std::string &button_name,
		const std::string &hover_event_name, const std::string &click_event_name) {
	// check if an element with given name already exists
	if (scenes[scene_name].buttons.find(button_name) != scenes[scene_name].buttons.end()) {
		SPDLOG_WARN("Button with name {} already exists in scene {}", button_name, scene_name);
		return scenes[scene_name].buttons[button_name];
	}
	scenes[scene_name].buttons[button_name] =
			UIButton(glm::vec3(0.0f), glm::vec2(0.0f), "", "", Handle<Texture>(-1), hover_event_name, click_event_name);
	return scenes[scene_name].buttons[button_name];
}

UIText &UIManager::add_ui_text(const std::string &scene_name, const std::string &text_name) {
	// check if an element with given name already exists
	if (scenes[scene_name].texts.find(text_name) != scenes[scene_name].texts.end()) {
		SPDLOG_WARN("Text with name {} already exists in scene {}", text_name, scene_name);
		return scenes[scene_name].texts[text_name];
	}
	scenes[scene_name].texts[text_name] = UIText();
	return scenes[scene_name].texts[text_name];
}

UISlider &UIManager::add_ui_slider(const std::string &scene_name, const std::string &lider_name) {
	// check if an element with given name already exists
	if (scenes[scene_name].sliders.find(lider_name) != scenes[scene_name].sliders.end()) {
		SPDLOG_WARN("Slider with name {} already exists in scene {}", lider_name, scene_name);
		return scenes[scene_name].sliders[lider_name];
	}
	scenes[scene_name].sliders[lider_name] = UISlider();
	return scenes[scene_name].sliders[lider_name];
}

void UIManager::add_as_root(const std::string &scene_name, const std::string &element_name) {
	// this works ASSUMING that no two elements from different categories have the same name.
	// todo: change if this happens to be problematic
	// check if element with given name exists in any maps
	if (scenes[scene_name].anchors.find(element_name) != scenes[scene_name].anchors.end()) {
		scenes[scene_name].roots[element_name] = &scenes[scene_name].anchors[element_name];
	} else if (scenes[scene_name].images.find(element_name) != scenes[scene_name].images.end()) {
		scenes[scene_name].roots[element_name] = &scenes[scene_name].images[element_name];
	} else if (scenes[scene_name].buttons.find(element_name) != scenes[scene_name].buttons.end()) {
		scenes[scene_name].roots[element_name] = &scenes[scene_name].buttons[element_name];
	} else if (scenes[scene_name].texts.find(element_name) != scenes[scene_name].texts.end()) {
		scenes[scene_name].roots[element_name] = &scenes[scene_name].texts[element_name];
	} else if (scenes[scene_name].sliders.find(element_name) != scenes[scene_name].sliders.end()) {
		scenes[scene_name].roots[element_name] = &scenes[scene_name].sliders[element_name];
	}
}

void UIManager::draw() {
	ZoneScopedNC("UIManager::draw()", 0xd459ce);
	text_draw.r_scene = render_scene;
	sprite_draw.r_scene = render_scene;
	for (auto &scene : scenes) {
		if (scene.second.is_active) {
			for (auto &root : scene.second.roots) {
				root.second->draw();
			}
		}
	}
}

void UIManager::reset_scenes() {
	while (!scenes.empty()) {
		scenes.erase(scenes.begin());
	}
}

// draws world-space ui objects. Use in separate draw_viewport()
void UIManager::draw_world_space_ui() {
	ZoneScopedNC("UIManager::draw_world_space_ui()", 0xd459ce);
	text_draw.r_scene = render_scene;
	sprite_draw.r_scene = render_scene;
	for (auto &scene : scenes) {
		if (scene.second.is_active) {
			for (auto &root : scene.second.roots) {
				if (!root.second->is_screen_space) {
					root.second->draw();
				}
			}
		}
	}
}

// draws screenspace ui objects. Use in one call after both draw_viewport()
void UIManager::draw_screen_space_ui() {
	ZoneScopedNC("UIManager::draw_world_space_ui()", 0xd459ce);
	text_draw.r_scene = render_scene;
	sprite_draw.r_scene = render_scene;
	for (auto &scene : scenes) {
		if (scene.second.is_active) {
			for (auto &root : scene.second.roots) {
				if (root.second->is_screen_space) {
					root.second->draw();
				}
			}
		}
	}
}

void UIManager::add_to_root(
		const std::string &scene_name, const std::string &element_name, const std::string &root_name) {
	// this works ASSUMING that no two elements from different categories have the same name.

	// check if element with given name exists in any maps

	if (scenes[scene_name].anchors.find(element_name) != scenes[scene_name].anchors.end()) {
		scenes[scene_name].anchors[root_name].add_child(scenes[scene_name].anchors[element_name]);
	} else if (scenes[scene_name].images.find(element_name) != scenes[scene_name].images.end()) {
		scenes[scene_name].anchors[root_name].add_child(scenes[scene_name].images[element_name]);
	} else if (scenes[scene_name].buttons.find(element_name) != scenes[scene_name].buttons.end()) {
		scenes[scene_name].anchors[root_name].add_child(scenes[scene_name].buttons[element_name]);
	} else if (scenes[scene_name].texts.find(element_name) != scenes[scene_name].texts.end()) {
		scenes[scene_name].anchors[root_name].add_child(scenes[scene_name].texts[element_name]);
	} else if (scenes[scene_name].sliders.find(element_name) != scenes[scene_name].sliders.end()) {
		scenes[scene_name].anchors[root_name].add_child(scenes[scene_name].sliders[element_name]);
	}
}

UIAnchor &UIManager::get_ui_anchor(const std::string &scene_name, const std::string &anchor_name) {
	if (scenes[scene_name].anchors.find(anchor_name) == scenes[scene_name].anchors.end()) {
		SPDLOG_WARN("Ui anchor {} not found in scene {}", anchor_name, scene_name);
	}

	return scenes[scene_name].anchors[anchor_name];
}

UIImage &UIManager::get_ui_image(const std::string &scene_name, const std::string &image_name) {
	if (scenes[scene_name].images.find(image_name) == scenes[scene_name].images.end()) {
		SPDLOG_WARN("Ui image {} not found in scene {}", image_name, scene_name);
	}
	return scenes[scene_name].images[image_name];
}

UIButton &UIManager::get_ui_button(const std::string &scene_name, const std::string &button_name) {
	if (scenes[scene_name].buttons.find(button_name) == scenes[scene_name].buttons.end()) {
		SPDLOG_WARN("Ui button {} not found in scene {}", button_name, scene_name);
	}
	return scenes[scene_name].buttons[button_name];
}

UIText &UIManager::get_ui_text(const std::string &scene_name, const std::string &text_name) {
	if (scenes[scene_name].texts.find(text_name) == scenes[scene_name].texts.end()) {
		SPDLOG_WARN("Ui text {} not found in scene {}", text_name, scene_name);
	}
	return scenes[scene_name].texts[text_name];
}

UISlider &UIManager::get_ui_slider(const std::string &scene_name, const std::string &slider_name) {
	if (scenes[scene_name].sliders.find(slider_name) == scenes[scene_name].sliders.end()) {
		SPDLOG_WARN("Ui slider {} not found in scene {}", slider_name, scene_name);
	}
	return scenes[scene_name].sliders[slider_name];
}
