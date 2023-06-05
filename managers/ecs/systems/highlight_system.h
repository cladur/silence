#ifndef SILENCE_HIGHLIGHT_SYSTEM_H
#define SILENCE_HIGHLIGHT_SYSTEM_H

#include "base_system.h"

class HighlightSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_HIGHLIGHT_SYSTEM_H