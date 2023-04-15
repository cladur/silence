#include "vk_scene.h"

#include "render_manager.h"
#include <vulkan/vulkan_enums.hpp>

void RenderScene::init() {
	forward_pass.type = MeshpassType::Forward;
	shadow_pass.type = MeshpassType::DirectionalShadow;
	transparent_forward_pass.type = MeshpassType::Transparency;
}

Handle<RenderObject> RenderScene::register_object(MeshObject *object) {
	RenderObject new_obj;
	new_obj.bounds = object->bounds;
	new_obj.transform_matrix = object->transform_matrix;
	new_obj.material = get_material_handle(object->material);
	new_obj.mesh_id = get_mesh_handle(object->mesh);
	new_obj.update_index = (uint32_t)-1;
	new_obj.custom_sort_key = object->custom_sort_key;
	new_obj.pass_indices.clear(-1);
	Handle<RenderObject> handle{};
	handle.handle = static_cast<uint32_t>(renderables.size());

	renderables.push_back(new_obj);

	if (object->b_draw_forward_pass) {
		if (object->material->original->pass_shaders[MeshpassType::Transparency]) {
			transparent_forward_pass.unbatched_objects.push_back(handle);
		}
		if (object->material->original->pass_shaders[MeshpassType::Forward]) {
			forward_pass.unbatched_objects.push_back(handle);
		}
	}
	if (object->b_draw_shadow_pass) {
		if (object->material->original->pass_shaders[MeshpassType::DirectionalShadow]) {
			shadow_pass.unbatched_objects.push_back(handle);
		}
	}

	update_object(handle);
	return handle;
}

void RenderScene::register_object_batch(MeshObject *first, uint32_t count) {
	renderables.reserve(count);

	for (uint32_t i = 0; i < count; i++) {
		register_object(&(first[i]));
	}
}

void RenderScene::update_transform(Handle<RenderObject> object_id, const glm::mat4 &local_to_world) {
	get_object(object_id)->transform_matrix = local_to_world;
	update_object(object_id);
}

void RenderScene::update_object(Handle<RenderObject> object_id) {
	auto &pass_indices = get_object(object_id)->pass_indices;
	if (pass_indices[MeshpassType::Forward] != -1) {
		Handle<PassObject> obj{};
		obj.handle = pass_indices[MeshpassType::Forward];

		forward_pass.objects_to_delete.push_back(obj);
		forward_pass.unbatched_objects.push_back(object_id);

		pass_indices[MeshpassType::Forward] = -1;
	}

	if (pass_indices[MeshpassType::DirectionalShadow] != -1) {
		Handle<PassObject> obj{};
		obj.handle = pass_indices[MeshpassType::DirectionalShadow];

		shadow_pass.objects_to_delete.push_back(obj);
		shadow_pass.unbatched_objects.push_back(object_id);

		pass_indices[MeshpassType::DirectionalShadow] = -1;
	}

	if (pass_indices[MeshpassType::Transparency] != -1) {
		Handle<PassObject> obj{};
		obj.handle = pass_indices[MeshpassType::Transparency];

		transparent_forward_pass.objects_to_delete.push_back(obj);
		transparent_forward_pass.unbatched_objects.push_back(object_id);

		pass_indices[MeshpassType::Transparency] = -1;
	}

	if (get_object(object_id)->update_index == (uint32_t)-1) {
		get_object(object_id)->update_index = static_cast<uint32_t>(dirty_objects.size());

		dirty_objects.push_back(object_id);
	}
}

void RenderScene::write_object(GPUObjectData *target, Handle<RenderObject> object_id) {
	RenderObject *renderable = get_object(object_id);
	GPUObjectData object{};

	object.model_matrix = renderable->transform_matrix;
	object.origin_rad = glm::vec4(renderable->bounds.origin, renderable->bounds.radius);
	object.extents = glm::vec4(renderable->bounds.extents, renderable->bounds.valid ? 1.f : 0.f);

	memcpy(target, &object, sizeof(GPUObjectData));
}

void RenderScene::fill_object_data(GPUObjectData *data) {
	for (int i = 0; i < renderables.size(); i++) {
		Handle<RenderObject> h{};
		h.handle = i;
		write_object(data + i, h);
	}
}

void RenderScene::fill_indirect_array(GPUIndirectObject *data, MeshPass &pass) {
	int data_index = 0;
	for (int i = 0; i < pass.batches.size(); i++) {
		auto batch = pass.batches[i];

		data[data_index].command.firstInstance = batch.first; //i;
		data[data_index].command.instanceCount = 0;
		data[data_index].command.firstIndex = get_mesh(batch.mesh_id)->first_index;
		data[data_index].command.vertexOffset = (int)get_mesh(batch.mesh_id)->first_vertex;
		data[data_index].command.indexCount = get_mesh(batch.mesh_id)->index_count;
		data[data_index].object_id = 0;
		data[data_index].batch_id = i;

		data_index++;
	}
}

void RenderScene::fill_instances_array(GPUInstance *data, MeshPass &pass) {
	int data_index = 0;
	for (int i = 0; i < pass.batches.size(); i++) {
		auto batch = pass.batches[i];

		for (int b = 0; b < batch.count; b++) {
			data[data_index].object_id = pass.get(pass.flat_batches[b + batch.first].object)->original.handle;
			data[data_index].batch_id = i;
			data_index++;
		}
	}
}

void RenderScene::clear_dirty_objects() {
	for (auto obj : dirty_objects) {
		get_object(obj)->update_index = (uint32_t)-1;
	}
	dirty_objects.clear();
}

void RenderScene::build_batches() {
	// TODO: Refresh passes asynchroniously
	refresh_pass(&forward_pass);
	// refresh_pass(&shadow_pass);
	// refresh_pass(&transparent_forward_pass);
}

void RenderScene::merge_meshes(RenderManager *manager) {
	size_t total_vertices = 0;
	size_t total_indices = 0;

	for (auto &m : meshes) {
		m.first_index = static_cast<uint32_t>(total_indices);
		m.first_vertex = static_cast<uint32_t>(total_vertices);

		total_vertices += m.vertex_count;
		total_indices += m.index_count;

		m.is_merged = true;
	}

	merged_vertex_buffer = manager->create_buffer("Merged vertex buffer", total_vertices * sizeof(Vertex),
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vma::MemoryUsage::eGpuOnly);

	merged_index_buffer = manager->create_buffer("Merged index buffer", total_indices * sizeof(uint32_t),
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vma::MemoryUsage::eGpuOnly);

	manager->main_deletion_queue.push_function([=, this]() {
		manager->allocator.destroyBuffer(merged_vertex_buffer.buffer, merged_vertex_buffer.allocation);
		manager->allocator.destroyBuffer(merged_index_buffer.buffer, merged_index_buffer.allocation);
	});

	manager->immediate_submit([&](vk::CommandBuffer cmd) {
		for (auto &m : meshes) {
			vk::BufferCopy vertex_copy;
			vertex_copy.dstOffset = m.first_vertex * sizeof(Vertex);
			vertex_copy.size = m.vertex_count * sizeof(Vertex);
			vertex_copy.srcOffset = 0;

			cmd.copyBuffer(m.original->vertex_buffer.buffer, merged_vertex_buffer.buffer, 1, &vertex_copy);

			vk::BufferCopy index_copy;
			index_copy.dstOffset = m.first_index * sizeof(uint32_t);
			index_copy.size = m.index_count * sizeof(uint32_t);
			index_copy.srcOffset = 0;

			cmd.copyBuffer(m.original->index_buffer.buffer, merged_index_buffer.buffer, 1, &index_copy);
		}
	});
}

void RenderScene::refresh_pass(MeshPass *pass) {
	pass->needs_indirect_refresh = true;
	pass->needs_instance_refresh = true;

	std::vector<uint32_t> new_objects;
	if (!pass->objects_to_delete.empty()) {
		//create the render batches so that then we can do the deletion on the flat-array directly

		std::vector<RenderScene::RenderBatch> deletion_batches;
		deletion_batches.reserve(new_objects.size());

		for (auto i : pass->objects_to_delete) {
			pass->reusable_objects.push_back(i);
			RenderScene::RenderBatch new_command{};

			auto obj = pass->objects[i.handle];
			new_command.object = i;

			uint64_t pipelinehash = std::hash<uint64_t>()((uint64_t)(VkPipeline)obj.material.shader_pass->pipeline);
			uint64_t sethash = std::hash<uint64_t>()((uint64_t)(VkDescriptorSet)obj.material.material_set);

			auto mathash = static_cast<uint32_t>(pipelinehash ^ sethash);

			uint32_t meshmat = uint64_t(mathash) ^ uint64_t(obj.mesh_id.handle);

			//pack mesh id and material into 64 bits
			new_command.sort_key = uint64_t(meshmat) | (uint64_t(obj.custom_key) << 32);

			pass->objects[i.handle].custom_key = 0;
			pass->objects[i.handle].material.shader_pass = nullptr;
			pass->objects[i.handle].mesh_id.handle = -1;
			pass->objects[i.handle].original.handle = -1;

			deletion_batches.push_back(new_command);
		}
		pass->objects_to_delete.clear();
		{
			std::sort(deletion_batches.begin(), deletion_batches.end(),
					[](const RenderScene::RenderBatch &A, const RenderScene::RenderBatch &B) {
						if (A.sort_key < B.sort_key) {
							return true;
						} else if (A.sort_key == B.sort_key) {
							return A.object.handle < B.object.handle;
						} else {
							return false;
						}
					});
		}
		{
			std::vector<RenderScene::RenderBatch> new_batches;
			new_batches.reserve(pass->flat_batches.size());

			{
				std::set_difference(pass->flat_batches.begin(), pass->flat_batches.end(), deletion_batches.begin(),
						deletion_batches.end(), std::back_inserter(new_batches),
						[](const RenderScene::RenderBatch &A, const RenderScene::RenderBatch &B) {
							if (A.sort_key < B.sort_key) {
								return true;
							} else if (A.sort_key == B.sort_key) {
								return A.object.handle < B.object.handle;
							} else {
								return false;
							}
						});
			}
			pass->flat_batches = std::move(new_batches);
		}
	}
	{
		new_objects.reserve(pass->unbatched_objects.size());
		for (auto o : pass->unbatched_objects) {
			RenderScene::PassObject new_object{};

			new_object.original = o;
			new_object.mesh_id = get_object(o)->mesh_id;

			//pack mesh id and material into 32 bits
			vk_util::Material *mt = get_material(get_object(o)->material);
			new_object.material.material_set = mt->pass_sets[pass->type];
			new_object.material.shader_pass = mt->original->pass_shaders[pass->type];
			new_object.custom_key = get_object(o)->custom_sort_key;

			uint32_t handle = -1;

			//reuse handle
			if (!pass->reusable_objects.empty()) {
				handle = pass->reusable_objects.back().handle;
				pass->reusable_objects.pop_back();
				pass->objects[handle] = new_object;
			} else {
				handle = pass->objects.size();
				pass->objects.push_back(new_object);
			}

			new_objects.push_back(handle);
			get_object(o)->pass_indices[pass->type] = static_cast<int32_t>(handle);
		}

		pass->unbatched_objects.clear();
	}

	std::vector<RenderScene::RenderBatch> new_batches;
	new_batches.reserve(new_objects.size());

	{
		for (auto i : new_objects) {
			{
				RenderScene::RenderBatch new_command{};

				auto obj = pass->objects[i];
				new_command.object.handle = i;

				uint64_t pipelinehash = std::hash<uint64_t>()((uint64_t)(VkPipeline)obj.material.shader_pass->pipeline);
				uint64_t sethash = std::hash<uint64_t>()((uint64_t)(VkDescriptorSet)obj.material.material_set);

				auto mathash = static_cast<uint32_t>(pipelinehash ^ sethash);

				uint32_t meshmat = uint64_t(mathash) ^ uint64_t(obj.mesh_id.handle);

				//pack mesh id and material into 64 bits
				new_command.sort_key = uint64_t(meshmat) | (uint64_t(obj.custom_key) << 32);

				new_batches.push_back(new_command);
			}
		}
	}

	{
		std::sort(new_batches.begin(), new_batches.end(),
				[](const RenderScene::RenderBatch &A, const RenderScene::RenderBatch &B) {
					if (A.sort_key < B.sort_key) {
						return true;
					} else if (A.sort_key == B.sort_key) {
						return A.object.handle < B.object.handle;
					} else {
						return false;
					}
				});
	}
	{
		//merge the new batches into the main batch array

		if (!pass->flat_batches.empty() && !new_batches.empty()) {
			size_t index = pass->flat_batches.size();
			pass->flat_batches.reserve(pass->flat_batches.size() + new_batches.size());

			for (auto b : new_batches) {
				pass->flat_batches.push_back(b);
			}

			RenderScene::RenderBatch *begin = pass->flat_batches.data();
			RenderScene::RenderBatch *mid = begin + index;
			RenderScene::RenderBatch *end = begin + pass->flat_batches.size();
			//std::sort(pass->flat_batches.begin(), pass->flat_batches.end(), [](const RenderScene::RenderBatch& A,
			//const RenderScene::RenderBatch& B) { 	return A.sort_key < B.sort_key;
			//	});
			std::inplace_merge(
					begin, mid, end, [](const RenderScene::RenderBatch &A, const RenderScene::RenderBatch &B) {
						if (A.sort_key < B.sort_key) {
							return true;
						} else if (A.sort_key == B.sort_key) {
							return A.object.handle < B.object.handle;
						} else {
							return false;
						}
					});
		} else if (pass->flat_batches.empty()) {
			pass->flat_batches = std::move(new_batches);
		}
	}

	{
		pass->batches.clear();

		build_indirect_batches(pass, pass->batches, pass->flat_batches);

		//flatten batches into multibatch
		Multibatch newbatch{};
		pass->multibatches.clear();

		newbatch.count = 1;
		newbatch.first = 0;

#if 1
		for (int i = 1; i < pass->batches.size(); i++) {
			IndirectBatch *joinbatch = &pass->batches[newbatch.first];
			IndirectBatch *batch = &pass->batches[i];

			bool b_compatible_mesh = get_mesh(joinbatch->mesh_id)->is_merged;

			bool b_same_mat = false;

			if (b_compatible_mesh && joinbatch->material.material_set == batch->material.material_set &&
					joinbatch->material.shader_pass == batch->material.shader_pass) {
				b_same_mat = true;
			}

			if (!b_same_mat || !b_compatible_mesh) {
				pass->multibatches.push_back(newbatch);
				newbatch.count = 1;
				newbatch.first = i;
			} else {
				newbatch.count++;
			}
		}
		pass->multibatches.push_back(newbatch);
#else
		for (int i = 0; i < pass->batches.size(); i++) {
			Multibatch newbatch;
			newbatch.count = 1;
			newbatch.first = i;
			pass->multibatches.push_back(newbatch);
		}
#endif
	}
}

void RenderScene::build_indirect_batches(
		MeshPass *pass, std::vector<IndirectBatch> &out_batches, std::vector<RenderScene::RenderBatch> &in_objects) {
	if (in_objects.empty()) {
		return;
	}

	RenderScene::IndirectBatch new_batch;
	new_batch.first = 0;
	new_batch.count = 0;

	new_batch.material = pass->get(in_objects[0].object)->material;
	new_batch.mesh_id = pass->get(in_objects[0].object)->mesh_id;

	out_batches.push_back(new_batch);
	RenderScene::IndirectBatch *back = &pass->batches.back();

	RenderScene::PassMaterial last_mat = pass->get(in_objects[0].object)->material;
	for (int i = 0; i < in_objects.size(); i++) {
		PassObject *obj = pass->get(in_objects[i].object);

		bool b_same_mesh = obj->mesh_id.handle == back->mesh_id.handle;
		bool b_same_material = false;
		if (obj->material == last_mat) {
			b_same_material = true;
		}

		if (!b_same_material || !b_same_mesh) {
			new_batch.material = obj->material;

			if (new_batch.material == back->material) {
				b_same_material = true;
			}
		}

		if (b_same_mesh && b_same_material) {
			back->count++;
		} else {
			new_batch.first = i;
			new_batch.count = 1;
			new_batch.mesh_id = obj->mesh_id;

			out_batches.push_back(new_batch);
			back = &out_batches.back();
		}
		//back->objects.push_back(obj->original);
	}
}

RenderObject *RenderScene::get_object(Handle<RenderObject> object_id) {
	return &renderables[object_id.handle];
}

DrawMesh *RenderScene::get_mesh(Handle<DrawMesh> object_id) {
	return &meshes[object_id.handle];
}

vk_util::Material *RenderScene::get_material(Handle<vk_util::Material> object_id) {
	return materials[object_id.handle];
}

RenderScene::MeshPass *RenderScene::get_mesh_pass(MeshpassType name) {
	switch (name) {
		case MeshpassType::Forward:
			return &forward_pass;
			break;
		case MeshpassType::Transparency:
			return &transparent_forward_pass;
			break;
		case MeshpassType::DirectionalShadow:
			return &shadow_pass;
			break;
		default:
			break;
	}
	return nullptr;
}
Handle<vk_util::Material> RenderScene::get_material_handle(vk_util::Material *m) {
	Handle<vk_util::Material> handle{};
	auto it = material_convert.find(m);
	if (it == material_convert.end()) {
		auto index = static_cast<uint32_t>(materials.size());
		materials.push_back(m);

		handle.handle = index;
		material_convert[m] = handle;
	} else {
		handle = (*it).second;
	}
	return handle;
}

Handle<DrawMesh> RenderScene::get_mesh_handle(Mesh *m) {
	Handle<DrawMesh> handle{};
	auto it = mesh_convert.find(m);
	if (it == mesh_convert.end()) {
		auto index = static_cast<uint32_t>(meshes.size());

		DrawMesh new_mesh{};
		new_mesh.original = m;
		new_mesh.first_index = 0;
		new_mesh.first_vertex = 0;
		new_mesh.vertex_count = static_cast<uint32_t>(m->vertices.size());
		new_mesh.index_count = static_cast<uint32_t>(m->indices.size());

		meshes.push_back(new_mesh);

		handle.handle = index;
		mesh_convert[m] = handle;
	} else {
		handle = (*it).second;
	}
	return handle;
}

RenderScene::PassObject *RenderScene::MeshPass::get(Handle<PassObject> handle) {
	return &objects[handle.handle];
}
