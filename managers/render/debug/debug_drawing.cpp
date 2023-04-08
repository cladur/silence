#include "debug_drawing.h"
#include <render/pipeline_builder.h>
#include <render/render_manager.h>
#include <render/vk_initializers.h>

extern RenderManager render_manager;

DebugDraw::DebugDraw() {
}

DebugDraw::~DebugDraw() {
}

void DebugDraw::draw_line(const glm::vec3 &from, const glm::vec3 &to) {
	Material *mat = render_manager.get_material("debug_material");

	RenderObject render_object;
	Mesh *line = render_manager.get_mesh("debug_line");

	render_object.mesh = line;
	render_object.material = mat;

	glm::mat4 transform = glm::mat4(1.0f);

	// this is a very hacky way to do this, but it works for now without having to update vertex buffers ️💀💀💀💀

	glm::vec3 p1 = line->vertices[0].position; // this is always (0, 0, 0) by how the mesh is generated
	glm::vec3 p2 = line->vertices[1].position; // this is always (1, 0, 0) by how the mesh is generated

	glm::vec3 to_1 = to - from;
	glm::vec3 n_p2 = glm::normalize(p2);
	glm::vec3 n_to_1 = glm::normalize(to_1);

	// Calculate the rotation angle and axis
	glm::vec3 axis = glm::cross(n_p2, n_to_1);
	axis = glm::normalize(axis);
	float angle = glm::dot(n_p2, n_to_1);
	angle = acos(angle / (glm::length(n_p2) * glm::length(n_to_1)));

	transform = glm::translate(transform, from);
	transform = glm::rotate(transform, angle, axis);
	transform = glm::scale(transform, glm::vec3(glm::distance(from, to), 1.0f, 1.0f));

	render_object.transform_matrix = transform;

	render_manager.renderables.push_back(render_object);
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

