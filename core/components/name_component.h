#ifndef SILENCE_NAME_H
#define SILENCE_NAME_H

struct Name {
public:
	std::string name;

	void serialize_json(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		obj["name"] = name;
		j.push_back(nlohmann::json::object());
		j.back()["name"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializer::get_data("name", j);
		name = obj["name"];
	}

	//constructor
	explicit Name(std::string name) : name(std::move(name)) {
	}

	Name() {
		name = "unnamed";
	}
};

#endif //SILENCE_NAME_H