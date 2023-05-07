#include "ui_draw.h"
#include <render/render_scene.h>

void UIDraw::draw() {
	int i = 0;
	for (UIElement *ui_object : ui_objects) {
		ui_object->draw(current_scene);
	}
}
