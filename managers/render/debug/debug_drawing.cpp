#include "debug_drawing.h"
#include <render/pipeline_builder.h>
#include <render/render_manager.h>
#include <render/vk_initializers.h>

extern RenderManager render_manager;

DebugDraw::DebugDraw() {
}

DebugDraw::~DebugDraw() {
}

void DebugDraw::draw_line(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &color) {

}

void DebugDraw::draw_box(const glm::vec3 &center, const glm::vec3 &scale) {

	Material *mat = render_manager.get_material("debug_material");

	RenderObject render_object;
    Mesh *box = render_manager.get_mesh("debug_box");

	render_object.mesh = box;
	render_object.material = mat;

	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::translate(transform, center);
	transform = glm::scale(transform, scale);

	render_object.transform_matrix = transform;

	render_manager.renderables.push_back(render_object);
}

void DebugDraw::draw_sphere(const glm::vec3 &center, float radius) {
    Material *mat = render_manager.get_material("debug_material");

    RenderObject render_object;
    Mesh *box = render_manager.get_mesh("debug_sphere");

    render_object.mesh = box;
    render_object.material = mat;

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, center);
    transform = glm::scale(transform, glm::vec3(radius));

    render_object.transform_matrix = transform;

    render_manager.renderables.push_back(render_object);
}

