#ifndef SILENCE_JSON_DESERIALIZER_H
#define SILENCE_JSON_DESERIALIZER_H

#include "core/class_map.h"

class JsonDeserializer {
public:
	template <typename T> void add_component_to_map(std::string name) {
		class_map[name] = &create_instance<T>;
	}

	void show_map() {
		for (auto &[name, component] : class_map) {
			SPDLOG_INFO("{}", name);
		}
	}

private:
	map_type class_map;
};

#endif //SILENCE_JSON_DESERIALIZER_H
