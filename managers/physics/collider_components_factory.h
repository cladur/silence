#ifndef SILENCE_COLLIDER_COMPONENTS_FACTORY_H
#define SILENCE_COLLIDER_COMPONENTS_FACTORY_H

#include "types.h"

class collider_components_factory {
public:
	template <typename T> static void add_collider_component(Entity entity, T collider_component);
};

#endif //SILENCE_COLLIDER_COMPONENTS_FACTORY_H
