#ifndef SILENCE_SYSTEM_H
#define SILENCE_SYSTEM_H

class World;

class BaseSystem {
public:
	std::set<Entity> entities;

	virtual void startup(World &world) = 0;
	virtual void update(World &world, float dt) = 0;
};

#endif //SILENCE_SYSTEM_H