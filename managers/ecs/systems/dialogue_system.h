#ifndef SILENCE_DIALOGUE_SYSTEM_H
#define SILENCE_DIALOGUE_SYSTEM_H

#include "base_system.h"
#include "fmod_studio.hpp"
#include <render/transparent_elements/ui/ui_elements/ui_text.h>

struct Sentence {
	std::string text;
	std::string text2;
	FMOD::Studio::EventInstance *audio;
	float duration;
	bool played = false;
};

struct Dialogue {
	std::vector<Sentence> sentences;
};

class DialogueSystem : public BaseSystem {
private:
	std::string ui_name;
	UIText *ui_dialogue_text;
	UIText *ui_dialogue_text2;

	bool first_frame = true;

	std::unordered_map<std::string, Dialogue> dialogues;

	float dialogue_timer = 0.0f;
	std::string current_dialogue_id;
	int current_sentence_id = 0;

public:
	~DialogueSystem();
	void startup(World &world) override;
	void update(World &world, float dt) override;

	void reset();
};

#endif //SILENCE_DIALOGUE_SYSTEM_H
