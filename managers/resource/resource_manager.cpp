#include "resource_manager.h"
ResourceManager &ResourceManager::get() {
	static ResourceManager instance;
	return instance;
}
void ResourceManager::startup() {
}
void ResourceManager::shutdown() {
}
