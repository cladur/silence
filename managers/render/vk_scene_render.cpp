#include "render/vk_types.h"
#include "render_manager.h"

#include "vk_initializers.h"
#include "vk_shaders.h"
#include <vulkan-memory-allocator-hpp/vk_mem_alloc_enums.hpp>
#include <vulkan/vulkan_enums.hpp>

AutoCVarFloat cvar_draw_distance("render.draw_distance", "Distance cull", 5000);

glm::vec4 normalize_plane(glm::vec4 p) {
	return p / glm::length(glm::vec3(p));
}

void RenderManager::execute_compute_cull(vk::CommandBuffer cmd, RenderScene::MeshPass &pass, CullParams &params) {
	if (pass.batches.empty()) {
		return;
	}

	vk::DescriptorBufferInfo object_buffer_info = render_scene.object_data_buffer.get_info();

	vk::DescriptorBufferInfo dynamic_info = get_current_frame().dynamic_data.source.get_info();
	dynamic_info.range = sizeof(GPUCameraData);

	vk::DescriptorBufferInfo instance_info = pass.pass_objects_buffer.get_info();

	vk::DescriptorBufferInfo final_info = pass.compacted_instance_buffer.get_info();

	vk::DescriptorBufferInfo indirect_info = pass.draw_indirect_buffer.get_info();

	vk::DescriptorImageInfo depth_pyramid_info = {};
	depth_pyramid_info.sampler = depth_sampler;
	depth_pyramid_info.imageView = depth_pyramid.default_view;
	depth_pyramid_info.imageLayout = vk::ImageLayout::eGeneral;

	vk::DescriptorSet comp_object_data_set;
	vk_util::DescriptorBuilder::begin(descriptor_layout_cache, get_current_frame().dynamic_descriptor_allocator)
			.bind_buffer(0, &object_buffer_info, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
			.bind_buffer(1, &indirect_info, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
			.bind_buffer(2, &instance_info, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
			.bind_buffer(3, &final_info, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
			.bind_image(4, &depth_pyramid_info, vk::DescriptorType::eCombinedImageSampler,
					vk::ShaderStageFlagBits::eCompute)
			.bind_buffer(5, &dynamic_info, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute)
			.build(comp_object_data_set);

	glm::mat4 projection = params.projmat;
	glm::mat4 projection_t = transpose(projection);

	glm::vec4 frustum_x = normalize_plane(projection_t[3] + projection_t[0]); // x + w < 0
	glm::vec4 frustum_y = normalize_plane(projection_t[3] + projection_t[1]); // y + w < 0

	DrawCullData cull_data = {};
	cull_data.p00 = projection[0][0];
	cull_data.p11 = projection[1][1];
	cull_data.znear = 0.1f;
	cull_data.zfar = params.draw_dist;
	cull_data.frustum[0] = frustum_x.x;
	cull_data.frustum[1] = frustum_x.z;
	cull_data.frustum[2] = frustum_y.y;
	cull_data.frustum[3] = frustum_y.z;
	cull_data.draw_count = static_cast<uint32_t>(pass.flat_batches.size());
	cull_data.culling_enabled = params.frustrum_cull;
	cull_data.lod_enabled = false;
	cull_data.occlusion_enabled = params.occlusion_cull;
	cull_data.lod_base = 10.f;
	cull_data.lod_step = 1.5f;
	cull_data.pyramid_width = static_cast<float>(depth_pyramid_width);
	cull_data.pyramid_height = static_cast<float>(depth_pyramid_height);
	cull_data.view_mat = params.viewmat; //get_view_matrix();

	cull_data.aabb_check = params.aabb;
	cull_data.aabb_min_x = params.aabb_min.x;
	cull_data.aabb_min_y = params.aabb_min.y;
	cull_data.aabb_min_z = params.aabb_min.z;

	cull_data.aabb_max_x = params.aabb_max.x;
	cull_data.aabb_max_y = params.aabb_max.y;
	cull_data.aabb_max_z = params.aabb_max.z;

	if (params.draw_dist > 10000) {
		cull_data.distance_check = false;
	} else {
		cull_data.distance_check = true;
	}

	cmd.bindPipeline(vk::PipelineBindPoint::eCompute, cull_pipeline);

	cmd.pushConstants(cull_layout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(DrawCullData), &cull_data);

	cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, cull_layout, 0, 1, &comp_object_data_set, 0, nullptr);

	cmd.dispatch(static_cast<uint32_t>((pass.flat_batches.size() / 256) + 1), 1, 1);

	//barrier the 2 buffers we just wrote for culling, the indirect draw one, and the instances one, so that they can be
	//read well when rendering the pass
	{
		vk::BufferMemoryBarrier barrier =
				vk_init::buffer_barrier(pass.compacted_instance_buffer.buffer, graphics_queue_family);
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eIndirectCommandRead;

		vk::BufferMemoryBarrier barrier2 =
				vk_init::buffer_barrier(pass.draw_indirect_buffer.buffer, graphics_queue_family);
		barrier2.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
		barrier2.dstAccessMask = vk::AccessFlagBits::eIndirectCommandRead;

		vk::BufferMemoryBarrier barriers[] = { barrier, barrier2 };

		post_cull_barriers.push_back(barrier);
		post_cull_barriers.push_back(barrier2);
	}
}

void RenderManager::ready_mesh_draw(vk::CommandBuffer cmd) {
	//upload object data to gpu
	if (render_scene.dirty_objects.size() > 0) {
		size_t copy_size = render_scene.renderables.size() * sizeof(GPUObjectData);
		if (render_scene.object_data_buffer.size < copy_size) {
			reallocate_buffer(render_scene.object_data_buffer, copy_size,
					vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer,
					vma::MemoryUsage::eCpuToGpu);
		}

		//if 80% of the objects are dirty, then just reupload the whole thing
		if (render_scene.dirty_objects.size() >= render_scene.renderables.size() * 0.8) {
			AllocatedBuffer<GPUObjectData> new_buffer = create_buffer("80% or more dirty", copy_size,
					vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer,
					vma::MemoryUsage::eCpuToGpu);

			GPUObjectData *object_ssbo = map_buffer(new_buffer);
			render_scene.fill_object_data(object_ssbo);
			unmap_buffer(new_buffer);

			get_current_frame().frame_deletion_queue.push_function(
					[=, this]() { allocator.destroyBuffer(new_buffer.buffer, new_buffer.allocation); });

			//copy from the uploaded cpu side instance buffer to the gpu one
			vk::BufferCopy indirect_copy;
			indirect_copy.dstOffset = 0;
			indirect_copy.size = render_scene.renderables.size() * sizeof(GPUObjectData);
			indirect_copy.srcOffset = 0;
			cmd.copyBuffer(new_buffer.buffer, render_scene.object_data_buffer.buffer, 1, &indirect_copy);
		} else {
			//update only the changed elements

			std::vector<vk::BufferCopy> copies;
			copies.reserve(render_scene.dirty_objects.size());

			uint64_t buffersize = sizeof(GPUObjectData) * render_scene.dirty_objects.size();
			uint64_t vec4size = sizeof(glm::vec4);
			uint64_t intsize = sizeof(uint32_t);
			uint64_t wordsize = sizeof(GPUObjectData) / sizeof(uint32_t);
			uint64_t uploadSize = render_scene.dirty_objects.size() * wordsize * intsize;
			AllocatedBuffer<GPUObjectData> new_buffer = create_buffer("New buffer, changed elements", buffersize,
					vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer,
					vma::MemoryUsage::eCpuToGpu);
			AllocatedBuffer<uint32_t> target_buffer = create_buffer("Target buffer, changed elements", uploadSize,
					vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer,
					vma::MemoryUsage::eCpuToGpu);

			get_current_frame().frame_deletion_queue.push_function([=, this]() {
				allocator.destroyBuffer(new_buffer.buffer, new_buffer.allocation);
				allocator.destroyBuffer(target_buffer.buffer, target_buffer.allocation);
			});

			uint32_t *target_data = map_buffer(target_buffer);
			GPUObjectData *object_ssbo = map_buffer(new_buffer);
			uint32_t launchcount = static_cast<uint32_t>(render_scene.dirty_objects.size() * wordsize);
			{
				uint32_t sidx = 0;
				for (int i = 0; i < render_scene.dirty_objects.size(); i++) {
					render_scene.write_object(object_ssbo + i, render_scene.dirty_objects[i]);

					uint32_t dstOffset = static_cast<uint32_t>(wordsize * render_scene.dirty_objects[i].handle);

					for (int b = 0; b < wordsize; b++) {
						uint32_t tidx = dstOffset + b;
						target_data[sidx] = tidx;
						sidx++;
					}
				}
				launchcount = sidx;
			}
			unmap_buffer(new_buffer);
			unmap_buffer(target_buffer);

			vk::DescriptorBufferInfo indexData = target_buffer.get_info();

			vk::DescriptorBufferInfo sourceData = new_buffer.get_info();

			vk::DescriptorBufferInfo targetInfo = render_scene.object_data_buffer.get_info();

			vk::DescriptorSet COMPObjectDataSet;
			vk_util::DescriptorBuilder::begin(descriptor_layout_cache, get_current_frame().dynamic_descriptor_allocator)
					.bind_buffer(0, &indexData, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
					.bind_buffer(1, &sourceData, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
					.bind_buffer(2, &targetInfo, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
					.build(COMPObjectDataSet);

			cmd.bindPipeline(vk::PipelineBindPoint::eCompute, sparse_upload_pipeline);

			cmd.pushConstants(
					sparse_upload_layout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(uint32_t), &launchcount);

			cmd.bindDescriptorSets(
					vk::PipelineBindPoint::eCompute, sparse_upload_layout, 0, 1, &COMPObjectDataSet, 0, nullptr);

			cmd.dispatch(((launchcount) / 256) + 1, 1, 1);
		}

		vk::BufferMemoryBarrier barrier =
				vk_init::buffer_barrier(render_scene.object_data_buffer.buffer, graphics_queue_family);
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;

		upload_barriers.push_back(barrier);
		render_scene.clear_dirty_objects();
	}

	RenderScene::MeshPass *passes[3] = { &render_scene.forward_pass, &render_scene.transparent_forward_pass,
		&render_scene.shadow_pass };
	for (int p = 0; p < 1; p++) {
		auto &pass = *passes[p];

		//reallocate the gpu side buffers if needed

		if (pass.draw_indirect_buffer.size < pass.batches.size() * sizeof(GPUIndirectObject)) {
			reallocate_buffer(pass.draw_indirect_buffer, pass.batches.size() * sizeof(GPUIndirectObject),
					vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer |
							vk::BufferUsageFlagBits::eIndirectBuffer,
					vma::MemoryUsage::eGpuOnly);
		}

		if (pass.compacted_instance_buffer.size < pass.flat_batches.size() * sizeof(uint32_t)) {
			reallocate_buffer(pass.compacted_instance_buffer, pass.flat_batches.size() * sizeof(uint32_t),
					vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer,
					vma::MemoryUsage::eGpuOnly);
		}

		if (pass.pass_objects_buffer.size < pass.flat_batches.size() * sizeof(GPUInstance)) {
			reallocate_buffer(pass.pass_objects_buffer, pass.flat_batches.size() * sizeof(GPUInstance),
					vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer,
					vma::MemoryUsage::eGpuOnly);
		}
	}

	std::vector<AllocatedBufferUntyped> unmaps;

	for (int p = 0; p < 1; p++) {
		RenderScene::MeshPass &pass = *passes[p];
		RenderScene::MeshPass *ppass = passes[p];

		RenderScene *p_scene = &render_scene;
		//if the pass has changed the batches, need to reupload them
		if (pass.needs_indirect_refresh && pass.batches.size() > 0) {
			AllocatedBuffer<GPUIndirectObject> new_buffer =
					create_buffer("Reupload batches, indirect", sizeof(GPUIndirectObject) * pass.batches.size(),
							vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer |
									vk::BufferUsageFlagBits::eIndirectBuffer,
							vma::MemoryUsage::eCpuToGpu);

			GPUIndirectObject *indirect = map_buffer(new_buffer);

			p_scene->fill_indirect_array(indirect, *ppass);
			//async_calls.push_back([&]() {
			//	render_scene.fill_indirectArray(indirect, pass);
			//});

			unmaps.push_back(new_buffer);

			//unmap_buffer(new_buffer);

			if (pass.clear_indirect_buffer.buffer) {
				AllocatedBufferUntyped deletionBuffer = pass.clear_indirect_buffer;
				//add buffer to deletion queue of this frame
				get_current_frame().frame_deletion_queue.push_function(
						[=, this]() { allocator.destroyBuffer(deletionBuffer.buffer, deletionBuffer.allocation); });
			}

			main_deletion_queue.push_function(
					[=, this]() { allocator.destroyBuffer(new_buffer.buffer, new_buffer.allocation); });

			pass.clear_indirect_buffer = new_buffer;
			pass.needs_indirect_refresh = false;
		}

		if (pass.needs_instance_refresh && pass.flat_batches.size() > 0) {
			AllocatedBuffer<GPUInstance> new_buffer =
					create_buffer("Reupload batches, instance refresh", sizeof(GPUInstance) * pass.flat_batches.size(),
							vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer,
							vma::MemoryUsage::eCpuToGpu);

			GPUInstance *instanceData = map_buffer(new_buffer);
			p_scene->fill_instances_array(instanceData, *ppass);
			unmaps.push_back(new_buffer);
			//render_scene.fill_instances_array(instanceData, pass);
			//unmap_buffer(new_buffer);

			get_current_frame().frame_deletion_queue.push_function(
					[=]() { allocator.destroyBuffer(new_buffer.buffer, new_buffer.allocation); });

			//copy from the uploaded cpu side instance buffer to the gpu one
			vk::BufferCopy indirect_copy;
			indirect_copy.dstOffset = 0;
			indirect_copy.size = pass.flat_batches.size() * sizeof(GPUInstance);
			indirect_copy.srcOffset = 0;
			cmd.copyBuffer(new_buffer.buffer, pass.pass_objects_buffer.buffer, 1, &indirect_copy);

			vk::BufferMemoryBarrier barrier =
					vk_init::buffer_barrier(pass.pass_objects_buffer.buffer, graphics_queue_family);
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;

			upload_barriers.push_back(barrier);

			pass.needs_instance_refresh = false;
		}
	}

	for (auto b : unmaps) {
		unmap_buffer(b);
	}
	cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, {}, 0, nullptr,
			static_cast<uint32_t>(upload_barriers.size()), upload_barriers.data(), 0, nullptr); //1, &readBarrier);
	upload_barriers.clear();
}

void RenderManager::draw_objects_forward(vk::CommandBuffer cmd, RenderScene::MeshPass &pass) {
	glm::mat4 view = camera->get_view_matrix();
	float aspect = (float)window_extent.width / (float)window_extent.height;

	glm::mat4 projection = glm::perspective(glm::radians(70.f), aspect, cvar_draw_distance.get(), 0.1f);

	//fill a GPU camera data struct
	GPUCameraData cam_data = {};
	cam_data.proj = projection;
	cam_data.view = view;
	cam_data.viewproj = projection * view;

	float framed = ((float)frame_number / 30.f);

	scene_parameters.ambient_color = { sin(framed), 0, cos(framed), 1 };

	//push data to dynmem
	uint32_t scene_data_offset = get_current_frame().dynamic_data.push(scene_parameters);

	uint32_t camera_data_offset = get_current_frame().dynamic_data.push(cam_data);

	vk::DescriptorBufferInfo object_buffer_info = render_scene.object_data_buffer.get_info();

	vk::DescriptorBufferInfo scene_info = get_current_frame().dynamic_data.source.get_info();
	scene_info.range = sizeof(GPUSceneData);

	vk::DescriptorBufferInfo cam_info = get_current_frame().dynamic_data.source.get_info();
	cam_info.range = sizeof(GPUCameraData);

	vk::DescriptorBufferInfo instance_info = pass.compacted_instance_buffer.get_info();

	vk::DescriptorImageInfo shadow_image_info;
	shadow_image_info.sampler = shadow_sampler;

	shadow_image_info.imageView = shadow_image.default_view;
	shadow_image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::DescriptorSet global_set;
	vk_util::DescriptorBuilder::begin(descriptor_layout_cache, get_current_frame().dynamic_descriptor_allocator)
			.bind_buffer(0, &cam_info, vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eVertex)
			.bind_buffer(1, &scene_info, vk::DescriptorType::eUniformBufferDynamic,
					vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
			.build(global_set);

	vk::DescriptorSet object_data_set;
	vk_util::DescriptorBuilder::begin(descriptor_layout_cache, get_current_frame().dynamic_descriptor_allocator)
			.bind_buffer(0, &object_buffer_info, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
			.bind_buffer(1, &instance_info, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
			.build(object_data_set);
	cmd.setDepthBias(0, 0, 0);

	std::vector<uint32_t> dynamic_offsets;
	dynamic_offsets.push_back(camera_data_offset);
	dynamic_offsets.push_back(scene_data_offset);
	execute_draw_commands(cmd, pass, object_data_set, dynamic_offsets, global_set);
}

void RenderManager::execute_draw_commands(vk::CommandBuffer cmd, RenderScene::MeshPass &pass,
		vk::DescriptorSet object_data_set, std::vector<uint32_t> dynamic_offsets, vk::DescriptorSet global_set) {
	if (pass.batches.size() > 0) {
		Mesh *last_mesh = nullptr;
		vk::Pipeline last_pipeline{ VK_NULL_HANDLE };
		vk::PipelineLayout last_layout{ VK_NULL_HANDLE };
		vk::DescriptorSet last_meterial_set{ VK_NULL_HANDLE };

		vk::DeviceSize offset = 0;
		cmd.bindVertexBuffers(0, 1, &render_scene.merged_vertex_buffer.buffer, &offset);

		cmd.bindIndexBuffer(render_scene.merged_index_buffer.buffer, 0, vk::IndexType::eUint32);

		for (int i = 0; i < pass.multibatches.size(); i++) {
			auto &multibatch = pass.multibatches[i];
			auto &instance_draw = pass.batches[multibatch.first];

			vk::Pipeline new_pipeline = instance_draw.material.shader_pass->pipeline;
			vk::PipelineLayout new_layout = instance_draw.material.shader_pass->layout;
			vk::DescriptorSet new_material_set = instance_draw.material.material_set;

			Mesh *draw_mesh = render_scene.get_mesh(instance_draw.mesh_id)->original;

			if (new_pipeline != last_pipeline) {
				last_pipeline = new_pipeline;
				cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, new_pipeline);
				cmd.bindDescriptorSets(
						vk::PipelineBindPoint::eGraphics, new_layout, 1, 1, &object_data_set, 0, nullptr);

				//update dynamic binds
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, new_layout, 0, 1, &global_set,
						dynamic_offsets.size(), dynamic_offsets.data());
			}
			if (new_material_set != last_meterial_set) {
				last_meterial_set = new_material_set;
				cmd.bindDescriptorSets(
						vk::PipelineBindPoint::eGraphics, new_layout, 2, 1, &new_material_set, 0, nullptr);
			}

			bool merged = render_scene.get_mesh(instance_draw.mesh_id)->is_merged;
			if (merged) {
				if (last_mesh != nullptr) {
					vk::DeviceSize offset = 0;
					cmd.bindVertexBuffers(0, 1, &render_scene.merged_vertex_buffer.buffer, &offset);

					cmd.bindIndexBuffer(render_scene.merged_index_buffer.buffer, 0, vk::IndexType::eUint32);
					last_mesh = nullptr;
				}
			} else if (last_mesh != draw_mesh) {
				//bind the mesh vertex buffer with offset 0
				vk::DeviceSize offset = 0;
				cmd.bindVertexBuffers(0, 1, &draw_mesh->vertex_buffer.buffer, &offset);

				if (draw_mesh->index_buffer.buffer) {
					cmd.bindIndexBuffer(draw_mesh->index_buffer.buffer, 0, vk::IndexType::eUint32);
				}
				last_mesh = draw_mesh;
			}

			cmd.drawIndexedIndirect(pass.draw_indirect_buffer.buffer, multibatch.first * sizeof(GPUIndirectObject),
					multibatch.count, sizeof(GPUIndirectObject));
		}
	}
}

// void RenderManager::draw_objects_shadow(vk::CommandBuffer cmd, RenderScene::MeshPass &pass) {
// 	glm::mat4 view = _mainLight.get_view();

// 	glm::mat4 projection = _mainLight.get_projection();

// 	GPUCameraData cam_data;
// 	cam_data.proj = projection;
// 	cam_data.view = view;
// 	cam_data.viewproj = projection * view;

// 	//push data to dynmem
// 	uint32_t camera_data_offset = get_current_frame().dynamic_data.push(cam_data);

// 	vk::DescriptorBufferInfo object_buffer_info = render_scene.object_data_buffer.get_info();

// 	vk::DescriptorBufferInfo cam_info = get_current_frame().dynamic_data.source.get_info();
// 	cam_info.range = sizeof(GPUCameraData);

// 	vk::DescriptorBufferInfo instanceInfo = pass.compacted_instance_buffer.get_info();

// 	vk::DescriptorSet global_set;
// 	vk_util::DescriptorBuilder::begin(descriptor_layout_cache, get_current_frame().dynamic_descriptor_allocator)
// 			.bind_buffer(0, &cam_info, vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eVertex)
// 			.build(global_set);

// 	vk::DescriptorSet object_data_set;
// 	vk_util::DescriptorBuilder::begin(descriptor_layout_cache, get_current_frame().dynamic_descriptor_allocator)
// 			.bind_buffer(0, &object_buffer_info, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
// 			.bind_buffer(1, &instanceInfo, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
// 			.build(object_data_set);

// 	cmd.setDepthBias(CVAR_ShadowBias.GetFloat(), 0, CVAR_SlopeBias.GetFloat());

// 	std::vector<uint32_t> dynamic_offsets;
// 	dynamic_offsets.push_back(camera_data_offset);

// 	execute_draw_commands(cmd, pass, object_data_set, dynamic_offsets, global_set);
// }

struct alignas(16) DepthReduceData {
	glm::vec2 image_size;
};

inline uint32_t get_group_count(uint32_t thread_count, uint32_t local_size) {
	return (thread_count + local_size - 1) / local_size;
}

void RenderManager::reduce_depth(vk::CommandBuffer cmd) {
	vk::ImageMemoryBarrier depth_read_barriers[] = {
		vk_init::image_barrier(depth_image.image, vk::AccessFlagBits::eDepthStencilAttachmentWrite,
				vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eDepthStencilAttachmentOptimal,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eDepth),
	};

	cmd.pipelineBarrier(vk::PipelineStageFlagBits::eLateFragmentTests, vk::PipelineStageFlagBits::eComputeShader,
			vk::DependencyFlagBits::eByRegion, 0, nullptr, 0, nullptr, 1, depth_read_barriers);

	cmd.bindPipeline(vk::PipelineBindPoint::eCompute, depth_reduce_pipeline);

	for (int32_t i = 0; i < depth_pyramid_levels; ++i) {
		vk::DescriptorImageInfo dst_target;
		dst_target.sampler = depth_sampler;
		dst_target.imageView = depth_pyramid_mips[i];
		dst_target.imageLayout = vk::ImageLayout::eGeneral;

		vk::DescriptorImageInfo source_target;
		source_target.sampler = depth_sampler;
		if (i == 0) {
			source_target.imageView = depth_image.default_view;
			source_target.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		} else {
			source_target.imageView = depth_pyramid_mips[i - 1];
			source_target.imageLayout = vk::ImageLayout::eGeneral;
		}

		vk::DescriptorSet depth_set;
		vk_util::DescriptorBuilder::begin(descriptor_layout_cache, get_current_frame().dynamic_descriptor_allocator)
				.bind_image(0, &dst_target, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
				.bind_image(
						1, &source_target, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute)
				.build(depth_set);

		cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, depth_reduce_layout, 0, 1, &depth_set, 0, nullptr);

		uint32_t level_width = depth_pyramid_width >> i;
		uint32_t level_height = depth_pyramid_height >> i;
		if (level_height < 1) {
			level_height = 1;
		}
		if (level_width < 1) {
			level_width = 1;
		}

		DepthReduceData reduce_data = { glm::vec2(level_width, level_height) };

		cmd.pushConstants(depth_reduce_layout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(reduce_data), &reduce_data);
		cmd.dispatch(get_group_count(level_width, 32), get_group_count(level_height, 32), 1);

		vk::ImageMemoryBarrier reduce_barrier = vk_init::image_barrier(depth_pyramid.image,
				vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eGeneral, vk::ImageAspectFlagBits::eColor);

		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader,
				vk::DependencyFlagBits::eByRegion, 0, nullptr, 0, nullptr, 1, &reduce_barrier);
	}

	vk::ImageMemoryBarrier depth_write_barrier =
			vk_init::image_barrier(depth_image.image, vk::AccessFlagBits::eShaderRead,
					vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
					vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal,
					vk::ImageAspectFlagBits::eDepth);

	cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eEarlyFragmentTests,
			vk::DependencyFlagBits::eByRegion, 0, nullptr, 0, nullptr, 1, &depth_write_barrier);
}
