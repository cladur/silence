#ifndef SILENCE_SCENEMANAGER_H
#define SILENCE_SCENEMANAGER_H

class SceneManager {
public:
	void load_scene(const std::string &scene_name);
	static nlohmann::json save_scene(const std::string &scene_name, const std::vector<Entity> &entities);
	static void save_json_to_file(const std::string &file_name, const nlohmann::json &json);
};

#endif //SILENCE_SCENEMANAGER_H
