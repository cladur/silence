#ifndef SILENCE_VK_SCENE_H
#define SILENCE_VK_SCENE_H

#include "render/material_system.h"
#include "vk_mesh.h"
#include "vk_types.h"

template <typename T> struct Handle {
	uint32_t handle;
};

struct MeshObject;
struct Mesh;
struct GPUObjectData;
class RenderManager;

namespace vk_util {
struct Material;
struct ShaderPass;
} //namespace vk_util

struct GPUIndirectObject {
	vk::DrawIndexedIndirectCommand command;
	uint32_t object_id;
	uint32_t batch_id;
};

struct DrawMesh {
	uint32_t first_vertex;
	uint32_t first_index;
	uint32_t index_count;
	uint32_t vertex_count;
	bool is_merged;

	Mesh *original;
};

struct RenderObject {
	Handle<DrawMesh> mesh_id{};
	Handle<vk_util::Material> material{};

	uint32_t update_index = 0U;
	uint32_t custom_sort_key{ 0 };

	vk_util::PerPassData<int32_t> pass_indices{};

	glm::mat4 transform_matrix{};

	RenderBounds bounds{};
};

struct GPUInstance {
	uint32_t object_id;
	uint32_t batch_id;
};

class RenderScene {
public:
	struct PassMaterial {
		vk::DescriptorSet material_set;
		vk_util::ShaderPass *shader_pass = nullptr;

		bool operator==(const PassMaterial &other) const {
			return material_set == other.material_set && shader_pass == other.shader_pass;
		}
	};
	struct PassObject {
		PassMaterial material;
		Handle<DrawMesh> mesh_id{};
		Handle<RenderObject> original{};
		int32_t built_batch = 0;
		uint32_t custom_key = 0U;
	};
	struct RenderBatch {
		Handle<PassObject> object;
		uint64_t sort_key;

		bool operator==(const RenderBatch &other) const {
			return object.handle == other.object.handle && sort_key == other.sort_key;
		}
	};
	struct IndirectBatch {
		Handle<DrawMesh> mesh_id{};
		PassMaterial material;
		uint32_t first = 0U;
		uint32_t count = 0U;
	};

	struct Multibatch {
		uint32_t first;
		uint32_t count;
	};

	struct MeshPass {
		// final draw-indirect segments
		std::vector<RenderScene::Multibatch> multibatches;
		// draw indirect batches
		std::vector<RenderScene::IndirectBatch> batches;
		// sorted list of objects in the pass
		std::vector<RenderScene::RenderBatch> flat_batches;

		//unsorted object data
		std::vector<PassObject> objects;
		//objects pending addition
		std::vector<Handle<RenderObject>> unbatched_objects;

		//indicides for the objects array that can be reused
		std::vector<Handle<PassObject>> reusable_objects;

		//objects pending removal
		std::vector<Handle<PassObject>> objects_to_delete;

		AllocatedBuffer<uint32_t> compacted_instance_buffer;
		AllocatedBuffer<GPUInstance> pass_objects_buffer;

		AllocatedBuffer<GPUIndirectObject> draw_indirect_buffer;
		AllocatedBuffer<GPUIndirectObject> clear_indirect_buffer;

		PassObject *get(Handle<PassObject> handle);

		MeshpassType type;

		bool needs_indirect_refresh = true;
		bool needs_instance_refresh = true;
	};

	void init();

	Handle<RenderObject> register_object(MeshObject *object);

	void register_object_batch(MeshObject *first, uint32_t count);

	void update_transform(Handle<RenderObject> object_id, const glm::mat4 &local_to_world);
	void update_object(Handle<RenderObject> object_id);

	void fill_object_data(GPUObjectData *data);
	void fill_indirect_array(GPUIndirectObject *data, MeshPass &pass);
	void fill_instances_array(GPUInstance *data, MeshPass &pass);

	void write_object(GPUObjectData *target, Handle<RenderObject> object_id);

	void clear_dirty_objects();

	void build_batches();

	void merge_meshes(RenderManager *manager);

	void refresh_pass(MeshPass *pass);

	void build_indirect_batches(
			MeshPass *pass, std::vector<IndirectBatch> &out_batches, std::vector<RenderScene::RenderBatch> &in_objects);
	RenderObject *get_object(Handle<RenderObject> object_id);
	DrawMesh *get_mesh(Handle<DrawMesh> object_id);

	vk_util::Material *get_material(Handle<vk_util::Material> object_id);

	std::vector<RenderObject> renderables;
	std::vector<DrawMesh> meshes;
	std::vector<vk_util::Material *> materials;

	std::vector<Handle<RenderObject>> dirty_objects;

	MeshPass *get_mesh_pass(MeshpassType name);

	MeshPass forward_pass;
	MeshPass transparent_forward_pass;
	MeshPass shadow_pass;

	std::unordered_map<vk_util::Material *, Handle<vk_util::Material>> material_convert;
	std::unordered_map<Mesh *, Handle<DrawMesh>> mesh_convert;

	Handle<vk_util::Material> get_material_handle(vk_util::Material *m);
	Handle<DrawMesh> get_mesh_handle(Mesh *m);

	AllocatedBuffer<Vertex> merged_vertex_buffer;
	AllocatedBuffer<uint32_t> merged_index_buffer;

	AllocatedBuffer<GPUObjectData> object_data_buffer;
};

#endif // SILENCE_VK_SCENE_H