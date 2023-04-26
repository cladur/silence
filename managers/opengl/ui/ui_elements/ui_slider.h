#ifndef SILENCE_UI_SLIDER_H
#define SILENCE_UI_SLIDER_H

#include "ui_element.h"

enum class SliderAlignment {
	LEFT_TO_RIGHT,
	RIGHT_TO_LEFT,
	TOP_TO_BOTTOM,
	BOTTOM_TO_TOP
};

class UISlider : public UIElement{
public:
	float value = 0.0f;
	float min = 0.0f;
	float max = 1.0f;
	bool is_stepped = false;
	SliderAlignment slider_alignment = SliderAlignment::LEFT_TO_RIGHT;

	UISlider(float value, float min, float max, bool is_stepped);
	~UISlider();

	void draw() override;
};

#endif //SILENCE_UI_SLIDER_H
