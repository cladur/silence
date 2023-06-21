#ifndef SILENCE_SHOW_TAGGABLE_SYSTEM_H
#define SILENCE_SHOW_TAGGABLE_SYSTEM_H

#include "base_system.h"
#include <ecs/world.h>
class ShowTaggable : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_SHOW_TAGGABLE_SYSTEM_H
