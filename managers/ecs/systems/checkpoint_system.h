#ifndef SILENCE_CHECKPOINT_SYSTEM_H
#define SILENCE_CHECKPOINT_SYSTEM_H

#include "base_system.h"
#include "components/checkpoint_component.h"

class CheckpointSystem : public BaseSystem {
	Checkpoint *current_checkpoint = nullptr;

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;

	void reset(World &world);
};

#endif //SILENCE_CHECKPOINT_SYSTEM_H
