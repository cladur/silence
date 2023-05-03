#ifndef SILENCE_MENU_DEMO_H
#define SILENCE_MENU_DEMO_H

#include "opengl/transparent_elements/ui/ui_elements/ui_anchor.h"
#include "opengl/transparent_elements/ui/ui_elements/ui_button.h"
#include "opengl/transparent_elements/ui/ui_elements/ui_element.h"

class MenuDemo {
	UIAnchor root;
	UIButton play_button;
	UIButton options_button;
	UIButton credits_button;

public:
	void draw();
	MenuDemo();
};

#endif //SILENCE_MENU_DEMO_H
