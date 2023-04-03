#ifndef SILENCE_SCENEMANAGER_H
#define SILENCE_SCENEMANAGER_H

class SceneManager {
public:
	void load_scene(std::string scene_name);
	std::string save_scene(std::string scene_name);
};

#endif //SILENCE_SCENEMANAGER_H
