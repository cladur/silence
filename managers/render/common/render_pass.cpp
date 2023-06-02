#include "render_pass.h"
#include "material.h"
#include "render/common/framebuffer.h"
#include "render/common/utils.h"
#include "render/render_manager.h"
#include "resource/resource_manager.h"
#include <render/transparent_elements/particle_data.h>
#include <render/transparent_elements/particle_manager.h>
#include <tracy/tracy.hpp>

AutoCVarFloat cvar_blur_radius("render.blur_radius", "blur radius", 0.001f, CVarFlags::EditFloatDrag);
AutoCVarInt cvar_use_bloom("render.use_bloom", "use bloom", 1, CVarFlags::EditCheckbox);
AutoCVarFloat cvar_bloom_strength("render.bloom_strength", "bloom strength", 0.04f, CVarFlags::EditFloatDrag);

AutoCVarFloat cvar_gamma("render.gamma", "gamma", 2.2f, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_dirt_strength("render.dirt_strength", "dirt strength", 0.075f, CVarFlags::EditFloatDrag);

AutoCVarFloat cvar_smooth("render.particle_smooth", "particle smooth", 0.0003f, CVarFlags::EditFloatDrag);

void PBRPass::startup() {
	material.startup();
}

void PBRPass::draw(RenderScene &scene) {
	ZoneScopedN("PBRPass::draw");
	ResourceManager &resource_manager = ResourceManager::get();
	material.bind_resources(scene);
	utils::render_quad();
}

void LightPass::startup() {
	material.startup();
}

void LightPass::draw(RenderScene &scene) {
	ZoneScopedN("LightPass::draw");
	ResourceManager &resource_manager = ResourceManager::get();
	material.bind_resources(scene);
	for (auto &cmd : scene.light_draw_commands) {
		Light &light = *cmd.light;
		Transform &transform = *cmd.transform;
		material.bind_light_resources(light, transform);
		utils::render_sphere();
	}
}

void AOPass::startup() {
	material.startup();
}

void AOPass::draw(RenderScene &scene) {
	ZoneScopedN("AOPass::draw");
	RenderManager &render_manager = RenderManager::get();
	material.bind_resources(scene);
	utils::render_quad();
}

void AOBlurPass::startup() {
	material.startup();
}

void AOBlurPass::draw(RenderScene &scene) {
	ZoneScopedN("AOBlurPass::draw");
	RenderManager &render_manager = RenderManager::get();
	material.bind_resources(scene);
	utils::render_quad();
}

void SkyboxPass::startup() {
	material.startup();
	skybox.startup();
	skybox.load_from_directory(asset_path("cubemaps/venice_sunset"));
}

void SkyboxPass::draw(RenderScene &scene) {
	ZoneScopedN("SkyboxPass::draw");
	material.bind_resources(scene);
	skybox.draw();
}

void GBufferPass::startup() {
	material.startup();
}

void GBufferPass::draw(RenderScene &scene) {
	ZoneScopedN("GBufferPass::draw");
	ResourceManager &resource_manager = ResourceManager::get();
	material.bind_resources(scene);
	for (auto &cmd : scene.draw_commands) {
		ModelInstance &instance = *cmd.model_instance;
		Transform &transform = *cmd.transform;
		material.bind_instance_resources(instance, transform);
		Model &model = resource_manager.get_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			if (mesh.fc_bounding_sphere.is_on_frustum(scene.frustum, transform, scene)) {
				material.bind_mesh_resources(mesh);
				mesh.draw();
			}
		}
	}

#ifdef WIN32
	material.bind_skinned_resources(scene);
	for (auto &cmd : scene.skinned_draw_commands) {
		SkinnedModelInstance &instance = *cmd.model_instance;
		Transform &transform = *cmd.transform;
		material.bind_instance_resources(instance, transform);
		SkinnedModel &model = resource_manager.get_skinned_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			material.bind_mesh_resources(mesh);
			mesh.draw();
		}
	}
#endif
}

const uint32_t VERTEX_COUNT = 4000;
const uint32_t INDEX_COUNT = 6000;

void TransparentPass::startup() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, VERTEX_COUNT * sizeof(TransparentVertex), nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_COUNT * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TransparentVertex), (void *)nullptr);
	// vertex color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
			1, 3, GL_FLOAT, GL_FALSE, sizeof(TransparentVertex), (void *)offsetof(TransparentVertex, color));
	// vertex uv
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TransparentVertex), (void *)offsetof(TransparentVertex, uv));
	// is screen space
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(
			3, 1, GL_INT, sizeof(TransparentVertex), (void *)offsetof(TransparentVertex, is_screen_space));

	glBindVertexArray(0);
	material.startup();
}

void TransparentPass::draw(RenderScene &scene) {
	ZoneScopedNC("TransparentPass::draw", 0xad074f);
	RenderManager &render_manager = RenderManager::get();

	glm::vec3 cam_pos = scene.camera_pos;

	// transparency sorting for world-space objects
	std::sort(scene.transparent_objects.begin(), scene.transparent_objects.end(),
			[cam_pos](const TransparentObject &a, const TransparentObject &b) {
				return glm::distance(cam_pos, a.position) > glm::distance(cam_pos, b.position);
			});

	material.bind_resources(scene);

	for (auto &object : scene.transparent_objects) {
		if (object.vertices[0].is_screen_space) {
			screen_space_objects.push_back(object);
			continue;
		}
		material.bind_object_resources(scene, object);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, object.vertices.size() * sizeof(TransparentVertex), &object.vertices[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, object.indices.size() * sizeof(uint32_t), &object.indices[0]);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, object.indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	scene.transparent_objects.clear();
}

void TransparentPass::draw_worldspace(RenderScene &scene) {
	ZoneScopedNC("TransparentPass::draw_worldspace", 0xad074f);

	glm::vec3 cam_pos = scene.camera_pos;

	// transparency sorting for world-space objects
	std::sort(world_space_objects.begin(), world_space_objects.end(),
			[cam_pos](const TransparentObject &a, const TransparentObject &b) {
				auto cam_vector = glm::normalize(cam_pos - a.position);
				auto val_a = glm::distance(cam_pos, a.position + (cam_vector * a.billboard_z_offset));
				auto val_b = glm::distance(cam_pos, b.position + (cam_vector * b.billboard_z_offset));
				return val_a > val_b;
			});

	material.bind_resources(scene);

	for (auto &object : world_space_objects) {
		material.bind_object_resources(scene, object);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, object.vertices.size() * sizeof(TransparentVertex), &object.vertices[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, object.indices.size() * sizeof(uint32_t), &object.indices[0]);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, object.indices.size(), GL_UNSIGNED_INT, nullptr);
	}
}

void TransparentPass::draw_screenspace(RenderScene &scene) {
	ZoneScopedNC("TransparentPass::draw_screenspace", 0xad074f);
	material.bind_resources(scene);

	// transparency sorting for screen-space objects
	std::sort(screen_space_objects.begin(), screen_space_objects.end(),
			[](const TransparentObject &a, const TransparentObject &b) {
				return a.vertices[0].position.z < b.vertices[0].position.z;
			});

	for (auto &object : screen_space_objects) {
		material.bind_object_resources(scene, object);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, object.vertices.size() * sizeof(TransparentVertex), &object.vertices[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, object.indices.size() * sizeof(uint32_t), &object.indices[0]);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, object.indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	glBindVertexArray(0);

	screen_space_objects.clear();
	world_space_objects.clear();
}

void TransparentPass::sort_objects(RenderScene &scene) {
	ZoneScopedNC("TransparentPass::sort_objects", 0xad074f);
	for (auto &object : scene.transparent_objects) {
		if (object.vertices[0].is_screen_space) {
			screen_space_objects.push_back(object);
		} else {
			world_space_objects.push_back(object);
		}
	}
	scene.transparent_objects.clear();
}

void CombinationPass::startup() {
	material.startup();
}

void CombinationPass::draw(RenderScene &scene) {
	ZoneScopedN("CombinationPass::draw");
	material.bind_resources(scene);
	utils::render_quad();
}

void BloomPass::startup() {
	material.startup();
	// change dirt texture here
	ResourceManager::get().load_texture(asset_path("lens_dirts/lens_dirt_2.ktx2").c_str());
	dirt_texture = ResourceManager::get().get_texture_handle("lens_dirt_2");
	dirt_offsets[0] = rand() % 4444 / 8888.0f;
	dirt_offsets[1] = rand() % 2222 / 4444.0f;
}

void BloomPass::draw(RenderScene &scene) {
	ZoneScopedN("BloomPass::draw");
	std::vector<BloomMip> &mips = scene.bloom_buffer.mips;
	static int offset = 0;

	// DOWNSAMPLING
	material.downsample.use();
	material.downsample.set_int("srcTexture", 0);
	material.downsample.set_vec2("srcResolution", scene.render_extent.x, scene.render_extent.y);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.combination_buffer.texture_id);

	for (auto &mip : mips) {
		glViewport(0, 0, mip.size.x, mip.size.y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture_id, 0);
		utils::render_quad();

		material.downsample.set_vec2("srcResolution", mip.size);
		glBindTexture(GL_TEXTURE_2D, mip.texture_id);
	}

	// UPSAMPLING
	material.bloom.use();
	material.bloom.set_int("srcTexture", 0);
	material.bloom.set_float("filterRadius", cvar_blur_radius.get());

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	for (int i = mips.size() - 1; i > 0; i--) {
		auto &mip = mips[i];
		auto &next_mip = mips[i - 1];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mip.texture_id);

		glViewport(0, 0, next_mip.size.x, next_mip.size.y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, next_mip.texture_id, 0);
		utils::render_quad();
	}
	glDisable(GL_BLEND);

	scene.render_framebuffer.bind();
	glViewport(0, 0, scene.render_extent.x, scene.render_extent.y);

	// combine shader
	material.shader.use();
	material.shader.set_int("scene", 0);
	material.shader.set_int("bloom_tex", 1);
	material.shader.set_int("dirt", 2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.combination_buffer.texture_id);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, mips[0].texture_id);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, ResourceManager::get().get_texture(dirt_texture).id);
	material.shader.set_int("use_bloom", cvar_use_bloom.get());
	material.shader.set_float("bloom_strength", cvar_bloom_strength.get());
	material.shader.set_float("gamma", cvar_gamma.get());
	material.shader.set_float("dirt_strength", cvar_dirt_strength.get());
	material.shader.set_float("aspect_ratio", (float)scene.render_extent.x / (float)scene.full_render_extent.x);
	if (scene.render_extent.x - scene.full_render_extent.x < 0.0f) {
		material.shader.set_float("dirt_offset", dirt_offsets[offset++ % 2]);
	} else {
		material.shader.set_float("dirt_offset", 0.0f);
	}
	utils::render_quad();
}

void ShadowPass::startup() {
	material.startup();
}

void ShadowPass::draw(RenderScene &scene) {
	ResourceManager &resource_manager = ResourceManager::get();
	material.bind_resources(scene);

	for (auto &light_cmd : scene.light_draw_commands) {
		Light &light = *light_cmd.light;
		if (!light.cast_shadow || light.type != LightType::DIRECTIONAL_LIGHT) {
			continue;
		}

		Transform &light_transform = *light_cmd.transform;
		material.bind_light_resources(light, light_transform);

		for (auto &cmd : scene.draw_commands) {
			ModelInstance &instance = *cmd.model_instance;
			if (!instance.in_shadow_pass) {
				continue;
			}
			Transform &transform = *cmd.transform;
			material.bind_instance_resources(instance, transform);
			Model &model = resource_manager.get_model(instance.model_handle);
			for (auto &mesh : model.meshes) {
				if (mesh.fc_bounding_sphere.is_on_frustum(scene.frustum, transform, scene)) {
					mesh.draw();
				}
			}
		}

#ifdef WIN32
		material.bind_skinned_resources(scene);
		for (auto &cmd : scene.skinned_draw_commands) {
			SkinnedModelInstance &instance = *cmd.model_instance;
			if (!instance.in_shadow_pass) {
				continue;
			}
			Transform &transform = *cmd.transform;
			material.bind_instance_resources(instance, transform);
			SkinnedModel &model = resource_manager.get_skinned_model(instance.model_handle);
			for (auto &mesh : model.meshes) {
				mesh.draw();
			}
		}
#endif
	}
}

void MousePickPass::startup() {
	material.startup();
}

void MousePickPass::draw(RenderScene &scene) {
	ZoneScopedN("MousePickPass::draw");
	ResourceManager &resource_manager = ResourceManager::get();
	material.bind_resources(scene);
	for (auto &cmd : scene.draw_commands) {
		ModelInstance &instance = *cmd.model_instance;
		Transform &transform = *cmd.transform;
		Entity entity = cmd.entity;
		material.bind_instance_resources(instance, transform, entity);
		Model &model = resource_manager.get_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			if (mesh.fc_bounding_sphere.is_on_frustum(scene.frustum, transform, scene)) {
				mesh.draw();
			}
		}
	}

#ifdef WIN32
	for (auto &cmd : scene.skinned_draw_commands) {
		SkinnedModelInstance &instance = *cmd.model_instance;
		Transform &transform = *cmd.transform;
		Entity entity = cmd.entity;
		material.bind_instance_resources(instance, transform, entity);
		SkinnedModel &model = resource_manager.get_skinned_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			mesh.draw();
		}
	}
#endif
}

void ParticlePass::startup() {
	// initialize vao vbo and ebo
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	// set up vertex attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void *) offsetof(ParticleVertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void *) offsetof(ParticleVertex, color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void *) offsetof(ParticleVertex, tex_coords));

	// buffer data for the first time
	glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleVertex) * 4, nullptr, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, nullptr, GL_DYNAMIC_DRAW);

	// buffer index data as well
	std::vector<uint32_t> indices;
	indices.reserve(6);
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(uint32_t), indices.data());

	// initialize SSBO
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

	ParticleSSBODataBlock ssbo_data = {};
	for (int i = 0; i < MAX_PARTICLES_PER_ENTITY; i++) {
		ssbo_data.position[i] = glm::vec4(0.0f);
		ssbo_data.rotation[i] = glm::mat4(1.0f);
		ssbo_data.colors[i] = glm::vec4(1.0f);
	}

	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssbo_data), &ssbo_data, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	material.startup();
}

void ParticlePass::draw(RenderScene &scene) {
	ZoneScopedN("ParticlePass::draw");
	auto &rm = ResourceManager::get();
	auto &pm = ParticleManager::get();
	material.bind_resources(scene);

	auto cam_pos = scene.camera_pos;

	static std::vector<ParticleSSBODataBlock> ssbo_data;
	ssbo_data.clear();
	ssbo_data.resize(MAX_PARTICLES_PER_ENTITY);

	// copy the map to a vector of pairs
	static std::vector<std::pair<Entity, std::array<ParticleData, MAX_PARTICLES_PER_ENTITY>>> particles;
	particles.clear();
	particles.reserve(pm.particles.size());
	for (auto &p : pm.particles) {
        particles.emplace_back(p);
    }
	// now SORT the vector by distance to camera so that the farthest particles are drawn first
	std::sort(particles.begin(), particles.end(), [&cam_pos](auto &a, auto &b) {
        return glm::distance2(cam_pos, a.second[0].entity_position) > glm::distance2(cam_pos, b.second[0].entity_position);
    });

	static int i;
	static int j;

	i = 0;
	// draw the particles per entity
	//std::cout << "rendering [" << particles.size() << "] entities that emit particles" << std::endl;
	for (auto &p : particles) {
		auto &entity_id = p.first;
		auto &particles_array = p.second;

		// SORT particle data by distance to camera
		std::sort(particles_array.begin(), particles_array.end(), [&cam_pos](auto &a, auto &b) {
            return glm::distance2(cam_pos, a.position) > glm::distance2(cam_pos, b.position);
        });

		j = 0;
		for (auto &particle : particles_array) {
			if (particle.active) {
				glm::mat4 rot = glm::mat4(1.0f);
				rot = glm::rotate(rot, glm::radians(particle.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
				std::cout << j ;
				ssbo_data[i].position[j] = glm::vec4(particle.position, particle.size);
				ssbo_data[i].rotation[j] = rot;
				ssbo_data[i].colors  [j] = particle.color;
				std::cout << " end." << std::endl;
				j++;
			}
			if (j >= MAX_PARTICLES_PER_ENTITY) {
				SPDLOG_WARN("Too many particles for entity {}, consider lowering the rate or lifetime of the particles", entity_id);
				break;
			}
		}
		//std::cout << "entity {" << i << "} has currently [" << j << "] particles alive" << std::endl;

		// check if there are any particles to draw
		if (j == 0) {
			i++;
			continue;
		}

		// all particles have the same texture
		if (particles_array[0].is_textured != 0) {
			auto tex = rm.get_texture(particles_array[0].tex);
			material.shader.set_int("particle_tex", 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex.id);
		}

		material.shader.set_int("depth", 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, scene.g_buffer.depth_texture_id);

		material.shader.set_vec2("screen_size", glm::vec2(scene.render_extent.x, scene.render_extent.y));
		material.shader.set_float("smooth_size", cvar_smooth.get());
		material.shader.set_float("far_sub_near", scene.camera_near_far.y - scene.camera_near_far.y);
		material.shader.set_float("far", scene.camera_near_far.y);
		material.shader.set_float("near", scene.camera_near_far.x);
		material.shader.set_int("is_textured", particles_array[0].is_textured);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ssbo_data[i]), &ssbo_data[i]); // THIS THROWS EXCEPTION

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(ParticleVertex), &pm.default_particle_vertices[0]);

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, j);

		i++;
    }
	glBindVertexArray(0);
}