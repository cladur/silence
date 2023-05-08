#ifndef SILENCE_RENDER_MANAGER_H
#define SILENCE_RENDER_MANAGER_H

#include "render_scene.h"

#include "common/framebuffer.h"
#include "common/material.h"
#include "common/mesh.h"
#include "common/model.h"
#include "common/render_pass.h"
#include "common/shader.h"
#include "common/texture.h"

#include "components/transform_component.h"
#include "debug/debug_draw.h"
#include "transparent_elements/text/text_draw.h"

#include "camera/camera.h"

#include "resource/resource_manager.h"

class RenderManager {
private:
public:
	// Render scenes
	std::vector<RenderScene> render_scenes;
	int displayed_scene = -1;

	static RenderManager &get();

	void startup();
	void shutdown();
	uint32_t create_render_scene();

	void draw();
	void draw_scene(RenderScene &render_scene);

	void resize_framebuffer(uint32_t width, uint32_t height);
};

#endif // SILENCE_RENDER_MANAGER_H