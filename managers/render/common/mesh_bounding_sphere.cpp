#include "mesh_bounding_sphere.h"
#include "components/transform_component.h"
#include "frustum.h"
#include "render/render_scene.h"

AutoCVarInt cvar_draw_bounding_spheres(
		"render.draw_bounding_spheres", "Draw bounding spheres", 0, CVarFlags::EditCheckbox);

bool MeshBoundingSphere::is_on_or_in_front_of_plane(const FrustumPlane &f_plane) const {
	auto distance = f_plane.get_distance(center);
	return distance > -radius;
}

bool MeshBoundingSphere::is_on_frustum(const Frustum frustum, Transform transform, RenderScene &scene) const {
	glm::vec3 glob_scale = transform.get_global_scale();

	glm::vec3 global_center = transform.get_global_model_matrix() * glm::vec4(center, 1.0f);

	float max_scale = std::max(glob_scale.x, std::max(glob_scale.y, glob_scale.z));

	MeshBoundingSphere sphere(global_center, radius * max_scale);

	if (cvar_draw_bounding_spheres.get()) {
		scene.debug_draw.draw_sphere(sphere.center, sphere.radius, glm::vec3(1.0f, 0.0f, 0.0f));
	}

	return (sphere.is_on_or_in_front_of_plane(frustum.top) && sphere.is_on_or_in_front_of_plane(frustum.bottom) &&
			sphere.is_on_or_in_front_of_plane(frustum.left) && sphere.is_on_or_in_front_of_plane(frustum.right) &&
			sphere.is_on_or_in_front_of_plane(frustum.near) && sphere.is_on_or_in_front_of_plane(frustum.far));
}