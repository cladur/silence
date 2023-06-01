#ifndef SILENCE_TAGGABLE_SYSTEM_H
#define SILENCE_TAGGABLE_SYSTEM_H

#include "base_system.h"
#include <ecs/world.h>
class TaggableSystem : public BaseSystem {
private:
	std::string ui_name;
	std::string tag_prefix;
	Handle<Texture> tag_texture;
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_TAGGABLE_SYSTEM_H
