#ifndef SILENCE_MENU_DEMO_H
#define SILENCE_MENU_DEMO_H

#include "opengl/transparent_elements/ui/ui_elements/ui_anchor.h"
#include "opengl/transparent_elements/ui/ui_elements/ui_button.h"
#include "opengl/transparent_elements/ui/ui_elements/ui_element.h"
#include "opengl/transparent_elements/ui/ui_elements/ui_image.h"
#include "opengl/transparent_elements/ui/ui_elements/ui_text.h"

class MenuDemo {
	UIAnchor root;
	UIButton play_button;
	UIButton options_button;
	UIButton credits_button;

	UIAnchor title_root;
	UIText title_text;

	UIAnchor credits_root;
	UIText credits_text;

	UIAnchor options_root;
	UIText master_volume;
	UIImage volume_meter[10];
	UIButton plus_button;
	UIButton minus_button;

	UIAnchor back_button_root;
	UIButton back_button;

	FMOD::Studio::Bus* master_bus = nullptr;

public:
	void draw();
	MenuDemo();
};

#endif //SILENCE_MENU_DEMO_H