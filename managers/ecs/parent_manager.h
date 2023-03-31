#ifndef SILENCE_PARENT_MANAGER_H
#define SILENCE_PARENT_MANAGER_H

class ParentManager {
public:
	static bool add_child(Entity parent, Entity child);
	static bool remove_child(Entity parent, Entity child);
};

#endif //SILENCE_PARENT_MANAGER_H
