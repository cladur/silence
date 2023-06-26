#ifndef SILENCE_DIALOGUE_SYSTEM_H
#define SILENCE_DIALOGUE_SYSTEM_H

#include "base_system.h"
#include <render/transparent_elements/ui/ui_elements/ui_text.h>

class DialogueSystem : public BaseSystem {
private:
	std::string ui_name;
	UIText *ui_dialogue_text;
	UIText *ui_dialogue_text2;

	bool first_frame = true;

	float dialogue_timer = 0.0f;
	int current_dialogue_id = -1;
	int current_sentence_id = 0;

public:
	~DialogueSystem();
	void startup(World &world) override;
	void update(World &world, float dt) override;

	void reset();
};

#endif //SILENCE_DIALOGUE_SYSTEM_H
