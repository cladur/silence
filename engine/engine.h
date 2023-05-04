#ifndef SILENCE_ENGINE_H
#define SILENCE_ENGINE_H

#include "scene.h"

class Engine {
public:
	bool should_run = true;
	bool show_cvar_editor = false;

	// Standard engine methods
	virtual void startup();
	virtual void shutdown();
	void run();
	void update(float dt);
	virtual void custom_update(float dt) = 0;

	// Scene management
	std::vector<std::unique_ptr<Scene>> scenes;
	uint32_t active_scene = 0;

	virtual void create_scene(const std::string &name);
	Scene &get_active_scene();
	uint32_t get_scene_index(const std::string &name);
	void set_active_scene(const std::string &name);
};

#endif // SILENCE_ENGINE_H