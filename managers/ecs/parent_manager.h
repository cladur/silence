#ifndef SILENCE_PARENT_MANAGER_H
#define SILENCE_PARENT_MANAGER_H

#include "types.h"
class ParentManager {
public:
	static bool add_children(Entity parent, Entity children);
	static bool remove_children(Entity parent, Entity children);
};

#endif //SILENCE_PARENT_MANAGER_H
