#include <utility>

#ifndef SILENCE_TYPES_H
#define SILENCE_TYPES_H

std::string asset_path(std::string_view path);
std::string shader_path(std::string_view path);

// ECS

// Entity = id
using Entity = std::uint32_t;
// ComponentType = id
using ComponentType = std::uint8_t;
const ComponentType MAX_COMPONENTS = 32;

using Signature = std::bitset<MAX_COMPONENTS>;
const Entity MAX_ENTITIES = 5000;
const Entity MAX_CHILDREN = 255;

// Serialization help

class Serializer {
public:
	static nlohmann::json get_data(const std::string &component_name, nlohmann::json &j) {
		nlohmann::json obj;
		for (auto &element : j) {
			if (element.find(component_name) != element.end()) {
				obj = element[component_name];
				break;
			}
		}
		return obj;
	}

	static std::string get_lowercase_struct_name(std::string type_name) {
		std::string new_name = std::move(type_name);
		// if typename starts with struct, remove it
		if (new_name.substr(0, 7) == "struct ") {
			new_name.erase(0, 7);
		}
		// Remove number prefix from type name
		while (new_name[0] >= '0' && new_name[0] <= '9') {
			new_name.erase(0, 1);
		}

		return new_name;
	}
};

#endif //SILENCE_TYPES_H
