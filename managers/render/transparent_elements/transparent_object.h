#ifndef SILENCE_TRANSPARENT_OBJECT_H
#define SILENCE_TRANSPARENT_OBJECT_H

#include "render/common/shader.h"
#include "resource/resource_manager.h"
#include <render/common/texture.h>

struct TransparentVertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 uv;
	int is_screen_space;
};

enum class TransparentType {
	TEXT,
	SPRITE,
};

struct TransparentObject {
	std::vector<TransparentVertex> vertices;
	std::vector<uint32_t> indices;
	glm::vec2 size;
	glm::vec3 position;
	glm::mat4 transform;
	bool billboard = false;
	TransparentType type;
	std::string texture_name; // used only for fonts now
	//Handle<Texture> texture; this cannot be here for no fdssdfasfdasdf reason
};


#endif //SILENCE_TRANSPARENT_OBJECT_H
