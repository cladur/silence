#include "ui_element.h"
#include <render/render_scene.h>

void UIElement::draw(RenderScene *scene) {

}

void UIElement::draw(RenderScene *scene, glm::vec3 parent_position, glm::vec2 parent_size) {

}

void UIElement::add_child(UIElement &child) {
	child.parent_position = position;
	children.push_back(&child);
}
