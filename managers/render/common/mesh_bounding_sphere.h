#ifndef SILENCE_MESH_BOUNDING_SPHERE_H
#define SILENCE_MESH_BOUNDING_SPHERE_H

struct FrustumPlane;
struct Frustum;
struct Transform;
struct RenderScene;

// for frustum culling
struct MeshBoundingSphere {
	glm::vec3 center;
	float radius;

	bool is_on_or_in_front_of_plane(const FrustumPlane &f_plane) const;

	[[nodiscard]] bool is_on_frustum(const Frustum frustum, Transform transform, RenderScene &scene) const;
};

#endif //SILENCE_MESH_BOUNDING_SPHERE_H
