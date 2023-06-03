#ifndef SILENCE_HIGHLIGHT_SYSTEM_H
#define SILENCE_HIGHLIGHT_SYSTEM_H

#include "base_system.h"

class HighlightSystem : public BaseSystem {
public:
	std::set<Entity> entities;

	virtual void startup(World &world) = 0;
	virtual void update(World &world, float dt) = 0;
};

#endif //SILENCE_HIGHLIGHT_SYSTEM_H