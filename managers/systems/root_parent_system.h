#ifndef SILENCE_ROOTPARENTSYSTEM_H
#define SILENCE_ROOTPARENTSYSTEM_H

#include "base_system.h"
#include <glm/fwd.hpp>
class RootParentSystem : public BaseSystem {
public:
	void startup();
	void update();

private:
	void RootParentSystem::update_children(Entity parent, glm::mat4 parent_model);
};

#endif //SILENCE_ROOTPARENTSYSTEM_H
