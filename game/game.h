#ifndef SILENCE_GAME_H
#define SILENCE_GAME_H

#include "engine/engine.h"

class Game : public Engine {
public:
	bool controlling_camera = false;

	void startup() override;
	void shutdown() override;
	void custom_update(float dt) override;
};

#endif //SILENCE_GAME_H