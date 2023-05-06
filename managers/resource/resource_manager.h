#ifndef SILENCE_RESOURCE_MANAGER_H
#define SILENCE_RESOURCE_MANAGER_H

class ResourceManager {
private:
public:
	static ResourceManager &get();

	void startup();
	void shutdown();
};

#endif //SILENCE_RESOURCE_MANAGER_H
