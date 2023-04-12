#ifndef SILENCE_UI_TEXT_COMPONENT_H
#define SILENCE_UI_TEXT_COMPONENT_H

#include <render/ui/text/font.h>
#include <glm/vec3.hpp>
#include <string>

struct UIText {
	int font_id;
	std::string text;
	glm::vec3 color;
	float scale;

    void serialize_json(nlohmann::json &j) {
        // TODO good serialization
        nlohmann::json::object_t obj;
        obj["font_id"] = font_id;
        obj["text"] = text;
        obj["color_r"] = color.x;
        obj["color_g"] = color.y;
        obj["color_b"] = color.z;
        obj["scale"] = scale;
        j.push_back(nlohmann::json::object());
        j.back()["ui_text"] = obj;
    }

    void deserialize_json(nlohmann::json &j) {
        nlohmann::json obj = Serializaer::get_data("mesh_instance", j);
        font_id = obj["font_id"];
        text = obj["text"];
        color.x = obj["color_r"];
        color.y = obj["color_g"];
        color.z = obj["color_b"];
        scale = obj["scale"];
    }

};

#endif //SILENCE_UI_TEXT_COMPONENT_H
