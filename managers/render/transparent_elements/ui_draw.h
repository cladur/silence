#ifndef SILENCE_UI_DRAW_H
#define SILENCE_UI_DRAW_H

#include "transparent_object.h"
#include <render/transparent_elements/ui/ui_elements/ui_element.h>

//class UIElement;

class UIDraw {
public:
	std::vector<UIElement*> ui_objects;
	RenderScene *current_scene;

	void draw();
};

#endif //SILENCE_UI_DRAW_H
