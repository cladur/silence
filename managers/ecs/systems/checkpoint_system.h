#ifndef SILENCE_CHECKPOINT_SYSTEM_H
#define SILENCE_CHECKPOINT_SYSTEM_H

#include "base_system.h"
#include <render/transparent_elements/ui/ui_elements/ui_text.h>

class CheckpointSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;

	void reset(World &world);
};

#endif //SILENCE_CHECKPOINT_SYSTEM_H
