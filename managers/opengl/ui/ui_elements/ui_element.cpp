#include "ui_element.h"
void UIElement::draw() {

}

void UIElement::draw(glm::vec3 parent_position, glm::vec2 parent_size) {

}

void UIElement::add_child(UIElement &child) {
	children.push_back(&child);
}
