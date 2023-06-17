#ifndef SILENCE_LIGHT_SWITCHER_SYSTEM_H
#define SILENCE_LIGHT_SWITCHER_SYSTEM_H

#include "base_system.h"
#include "components/light_switcher_component.h"

class LightSwitcherSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
	void calculate_next_switch(LightSwitcher &light_switcher);
	void switch_light(World &world, const Entity &entity, LightSwitcher &light_switcher);
};

#endif //SILENCE_LIGHT_SWITCHER_SYSTEM_H
