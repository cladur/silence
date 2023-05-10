#ifndef SILENCE_CAMERA_H
#define SILENCE_CAMERA_H

struct Camera {
public:
	float fov = 70.0f;

	void serialize_json(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		obj["fov"] = fov;
		j.push_back(nlohmann::json::object());
		j.back()["camera"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializer::get_data("camera", j);
		fov = obj["fov"];
	}
};

#endif //SILENCE_NAME_H