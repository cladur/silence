#include "material.h"
#include "components/decal_component.h"
#include "display/display_manager.h"
#include "render/common/skinned_mesh.h"
#include "render/render_scene.h"
#include "render_pass.h"
#include <spdlog/spdlog.h>

AutoCVarInt cvar_use_ao("render.use_ao", "use ambient occlusion", 1, CVarFlags::EditCheckbox);
AutoCVarInt cvar_use_fog("render.use_fog", "use simple linear fog", 1, CVarFlags::EditCheckbox);
AutoCVarFloat cvar_fog_min("render.fog_min", "fog min distance", 40.0f, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_fog_max("render.fog_max", "fog max distance", 300.0f, CVarFlags::EditFloatDrag);
AutoCVarInt cvar_volumetric_steps(
		"render.volumetric_steps", "Set steps of volumetric calculation", 10, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_volumetric_scattering(
		"render.volumetric_scattering", "Set volumetric scattering value", 0.25f, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_volumetric_bias_x(
		"render.volumetric_bias_x", "Volumetric bias value to avoid acne", 0.0000001f, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_volumetric_bias_y(
		"render.volumetric_bias_y", "Volumetric bias value to avoid acne", 0.0000005f, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_bias_min("render.light_bias_min", "Spot light bias min", 0.0000001f);
AutoCVarFloat cvar_bias_max("render.light_bias_max", "Spot light bias max", 0.00000002f);
AutoCVarFloat cvar_ambient_strength(
		"render.ambient_strength", "how much light from skymap to use", 1.0f, CVarFlags::EditFloatDrag);

void MaterialPBR::startup() {
	shader.load_from_files(shader_path("pbr.vert"), shader_path("pbr.frag"));
}

void MaterialPBR::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_mat4("view", scene.view);
	shader.set_vec3("camPos", scene.camera_pos);
	shader.set_int("gPosition", 0);
	shader.set_int("gNormal", 1);
	shader.set_int("gAlbedo", 2);
	shader.set_int("gAoRoughMetal", 3);

	shader.set_int("irradiance_map", 5);
	shader.set_int("prefilter_map", 6);
	shader.set_int("brdf_lut", 7);

	shader.set_float("ambient_strength", cvar_ambient_strength.get());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.position_texture_id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.normal_texture_id);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.albedo_texture_id);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.ao_rough_metal_texture_id);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox_pass.skybox.irradiance_map.id);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox_pass.skybox.prefilter_map.id);

	glActiveTexture(GL_TEXTURE7);
	// TODO: Use baked brdf lut instead (it's broken atm)
	glBindTexture(GL_TEXTURE_2D, scene.skybox_pass.skybox.brdf_lut_texture);
}

void MaterialPBR::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialPBR::bind_mesh_resources(Mesh &mesh) {
}

void MaterialLight::startup() {
	shader.load_from_files(shader_path("light.vert"), shader_path("light.frag"));
}

void MaterialLight::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_mat4("projection", scene.projection);
	shader.set_mat4("view", scene.view);
	shader.set_vec3("camPos", scene.camera_pos);
	shader.set_vec2("screen_dimensions", scene.render_extent);
	shader.set_float("far_plane", scene.shadow_buffer.far);
	shader.set_int("gPosition", 0);
	shader.set_int("gNormal", 1);
	shader.set_int("gAlbedo", 2);
	shader.set_int("gAoRoughMetal", 3);
	shader.set_int("shadowMap", 4);
	shader.set_int("depthMap", 5);
	shader.set_int("gDepth", 6);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.position_texture_id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.normal_texture_id);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.albedo_texture_id);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.ao_rough_metal_texture_id);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.depth_texture_id);
}

void MaterialLight::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialLight::bind_light_resources(Light &light, Transform &transform) {
	float threshold = *CVarSystem::get()->get_float_cvar("render.light_threshold");
	// float radius = light.intensity * std::sqrtf(1.0f / threshold);

	const glm::mat4 &global_model = transform.get_global_model_matrix();
	glm::mat4 model;
	if (light.type == LightType::SPOT_LIGHT) {
		float cone_scale = light.radius * glm::tan(glm::radians(light.outer_cutoff + light.cutoff));
		model = glm::scale(global_model, glm::vec3(cone_scale, cone_scale, light.radius));
	} else {
		model = global_model * glm::scale(glm::mat4(1.0f), glm::vec3(light.radius));
	}
	shader.use();
	shader.set_mat4("model", model);
	shader.set_vec3("light_color", light.color);
	shader.set_float("light_intensity", light.intensity);
	shader.set_float("light_radius", light.radius);
	shader.set_float("light_blend_distance", light.blend_distance);
	shader.set_int("type", (int)light.type);
	shader.set_bool("cast_shadow", light.cast_shadow);
	shader.set_bool("cast_volumetric", light.cast_volumetric);
	switch (light.type) {
		case LightType::POINT_LIGHT:
			shader.set_vec3("light_position", transform.get_global_position());
			if (light.cast_shadow) {
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_CUBE_MAP, light.shadow_map_id);
			}
			break;
		case LightType::DIRECTIONAL_LIGHT:
			shader.set_vec3("light_direction",
					glm::normalize(transform.get_global_orientation() * glm::vec3(0.0f, 0.0f, -1.0f)));
			if (light.cast_shadow) {
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, light.shadow_map_id);
				shader.set_mat4("light_space", light.light_space);
			}
			if (light.cast_volumetric) {
				shader.set_vec2("spot_bias", glm::vec2(cvar_bias_min.get(), cvar_bias_max.get()));
				shader.set_vec2(
						"volumetric_bias", glm::vec2(cvar_volumetric_bias_x.get(), cvar_volumetric_bias_y.get()));
				shader.set_float("scattering", cvar_volumetric_scattering.get());
				shader.set_int("num_steps", cvar_volumetric_steps.get());
			}
			break;
		case LightType::SPOT_LIGHT:
			shader.set_vec3("light_position", transform.get_global_position());
			shader.set_vec3("light_direction",
					glm::normalize(transform.get_global_orientation() * glm::vec3(0.0f, 0.0f, -1.0f)));
			shader.set_float("cutoff", glm::cos(glm::radians(light.cutoff)));
			shader.set_float("outer_cutoff", glm::cos(glm::radians(light.outer_cutoff + light.cutoff)));
			if (light.cast_shadow) {
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, light.shadow_map_id);
				shader.set_mat4("light_space", light.light_space);
			}
			if (light.cast_volumetric) {
				shader.set_vec2("spot_bias", glm::vec2(cvar_bias_min.get(), cvar_bias_max.get()));
				shader.set_vec2(
						"volumetric_bias", glm::vec2(cvar_volumetric_bias_x.get(), cvar_volumetric_bias_y.get()));
				shader.set_float("scattering", cvar_volumetric_scattering.get());
				shader.set_int("num_steps", cvar_volumetric_steps.get());
			}
			break;
		default:
			SPDLOG_WARN("Invalid light type!");
			break;
	}
}

float our_lerp(float a, float b, float f) {
	return a + f * (b - a);
}

void MaterialAO::startup() {
	shader.load_from_files(shader_path("ssao.vert"), shader_path("ssao.frag"));

	std::uniform_real_distribution<GLfloat> rand_float(0.0, 1.0);
	std::default_random_engine gen;
	std::vector<glm::vec3> noise_tex;

	// ssao kernel
	for (unsigned int i = 0; i < 64; ++i) {
		glm::vec3 sample(rand_float(gen) * 2.0 - 1.0, rand_float(gen) * 2.0 - 1.0, rand_float(gen));
		sample = glm::normalize(sample);
		sample *= rand_float(gen);
		float scale = float(i) / 64.0f;

		scale = our_lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssao_kernel.push_back(sample);
	}

	// noise texture
	for (unsigned int i = 0; i < 16; i++) {
		glm::vec3 noise(rand_float(gen), rand_float(gen), 0.0f);
		noise = glm::normalize(noise);
		noise_tex.push_back(noise);
	}

	glGenTextures(1, &noise_texture_id);
	glBindTexture(GL_TEXTURE_2D, noise_texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &noise_tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void MaterialAO::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_int("gPosition", 0);
	shader.set_int("gNormal", 1);
	shader.set_int("texNoise", 2);
	shader.set_mat4("projection", scene.projection);
	shader.set_mat4("view", scene.view);
	for (unsigned int i = 0; i < 64; i++) {
		shader.set_vec3("samples[" + std::to_string(i) + "]", ssao_kernel[i]);
	}
	shader.set_int("kernel_size", 64);
	shader.set_float("radius", radius);
	shader.set_float("bias", bias);
	shader.set_float("noise_size", 4.0f);
	shader.set_vec2("screen_dimensions", glm::vec2(scene.render_extent.x, scene.render_extent.y));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.position_texture_id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.normal_texture_id);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noise_texture_id);
}

void MaterialAO::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialAOBlur::startup() {
	shader.load_from_files(shader_path("ssao.vert"), shader_path("ssao_blur.frag"));
}

void MaterialAOBlur::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_int("ssao_texture", 0);
	shader.set_vec2("offset_step", glm::vec2(1.0f / scene.render_extent.x, 1.0f / scene.render_extent.y));
	shader.set_int("should_blur", should_blur);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.ssao_buffer.ssao_texture_id);
}

void MaterialAOBlur::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialGBuffer::startup() {
	shader.load_from_files(shader_path("gbuffer/gbuffer.vert"), shader_path("gbuffer/gbuffer.frag"));
#ifdef WIN32
	skinned_shader.load_from_files(shader_path("gbuffer/skinned_gbuffer.vert"), shader_path("gbuffer/gbuffer.frag"));
#endif
}

void MaterialGBuffer::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_mat4("view", scene.view);
	shader.set_mat4("projection", scene.projection);
	shader.set_vec3("camPos", scene.camera_pos);
	shader.set_int("albedo_map", 0);
	shader.set_int("normal_map", 1);
	shader.set_int("ao_metallic_roughness_map", 2);
	shader.set_int("emissive_map", 3);

	shader.set_int("irradiance_map", 5);
	shader.set_int("prefilter_map", 6);
	shader.set_int("brdf_lut", 7);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox_pass.skybox.irradiance_map.id);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox_pass.skybox.prefilter_map.id);

	glActiveTexture(GL_TEXTURE7);
	// TODO: Use baked brdf lut instead (it's broken atm)
	glBindTexture(GL_TEXTURE_2D, scene.skybox_pass.skybox.brdf_lut_texture);
}

void MaterialGBuffer::bind_skinned_resources(RenderScene &scene) {
	skinned_shader.use();
	skinned_shader.set_mat4("view", scene.view);
	skinned_shader.set_mat4("projection", scene.projection);
	skinned_shader.set_int("albedo_map", 0);
	skinned_shader.set_int("normal_map", 1);
	skinned_shader.set_int("ao_metallic_roughness_map", 2);
	skinned_shader.set_int("emissive_map", 3);

	skinned_shader.set_int("irradiance_map", 5);
	skinned_shader.set_int("prefilter_map", 6);
	skinned_shader.set_int("brdf_lut", 7);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox_pass.skybox.irradiance_map.id);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox_pass.skybox.prefilter_map.id);

	glActiveTexture(GL_TEXTURE7);
	// TODO: Use baked brdf lut instead (it's broken atm)
	glBindTexture(GL_TEXTURE_2D, scene.skybox_pass.skybox.brdf_lut_texture);
}

void MaterialGBuffer::bind_instance_resources(ModelInstance &instance, Transform &transform) {
	shader.set_mat4("model", transform.get_global_model_matrix());
	shader.set_bool("flip_uv_y", instance.flip_uv_y);
	if (!instance.scale_uv_with_transform) {
		shader.set_vec2("uv_scale", glm::vec2(1.0f));
		return;
	}
	glm::vec2 uv_scale = instance.uv_scale == glm::vec2(1.0f) ? transform.get_global_scale() : instance.uv_scale;
	shader.set_vec2("uv_scale", uv_scale);
}

void MaterialGBuffer::bind_mesh_resources(Mesh &mesh) {
	for (int i = 0; i < mesh.textures.size(); i++) {
		if (mesh.textures_present[i]) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
		}
	}
	// We flip the bools here, to force the update of uniforms on the first mesh
	static bool is_ao_map_set = !mesh.has_ao_map;
	static bool is_emissive_map_set = !mesh.textures_present[3];
	static bool is_normal_map_set = !mesh.textures_present[1];
	if (is_ao_map_set != mesh.has_ao_map) {
		shader.set_bool("has_ao_map", mesh.has_ao_map);
		is_ao_map_set = mesh.has_ao_map;
	}
	if (is_normal_map_set != mesh.has_normal_map) {
		shader.set_bool("has_normal_map", mesh.has_normal_map);
		is_normal_map_set = mesh.has_normal_map;
	}
	if (is_emissive_map_set != mesh.textures_present[3]) {
		shader.set_bool("has_emissive_map", mesh.textures_present[3]);
		is_emissive_map_set = mesh.textures_present[3];
	}
}

void MaterialGBuffer::bind_instance_resources(SkinnedModelInstance &instance, Transform &transform) {
	skinned_shader.set_mat4("model", transform.get_global_model_matrix());
	//TODO: make this functionality in shader function
	glBindBuffer(GL_UNIFORM_BUFFER, instance.skinning_buffer);
	if (!instance.bone_matrices.empty()) {
		glBufferSubData(
				GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * instance.bone_matrices.size(), instance.bone_matrices.data());
	}

	GLuint binding_index = 1;
	GLuint buffer_index = glGetUniformBlockIndex(skinned_shader.id, "SkinningBuffer");
	glUniformBlockBinding(skinned_shader.id, buffer_index, binding_index);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding_index, instance.skinning_buffer);

	skinned_shader.set_vec2("uv_scale", glm::vec2(1.0f));
}

void MaterialGBuffer::bind_mesh_resources(SkinnedMesh &mesh) {
	for (int i = 0; i < mesh.textures.size(); i++) {
		if (mesh.textures_present[i]) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
		}
	}
	// We flip the bools here, to force the update of uniforms on the first mesh
	static bool is_ao_map_set = !mesh.has_ao_map;
	static bool is_emissive_map_set = !mesh.textures_present[3];
	static bool is_normal_map_set = !mesh.textures_present[1];
	if (is_ao_map_set != mesh.has_ao_map) {
		skinned_shader.set_bool("has_ao_map", mesh.has_ao_map);
		is_ao_map_set = mesh.has_ao_map;
	}
	if (is_normal_map_set != mesh.has_normal_map) {
		skinned_shader.set_bool("has_normal_map", mesh.has_normal_map);
		is_normal_map_set = mesh.has_normal_map;
	}
	if (is_emissive_map_set != mesh.textures_present[3]) {
		skinned_shader.set_bool("has_emissive_map", mesh.textures_present[3]);
		is_emissive_map_set = mesh.textures_present[3];
	}
}

void MaterialSkybox::startup() {
	shader.load_from_files(shader_path("cubemap.vert"), shader_path("skybox.frag"));
}

void MaterialSkybox::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_int("environment_map", 0);
	shader.set_mat4("view", scene.view);
	shader.set_mat4("projection", scene.projection);
}

void MaterialSkybox::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialTransparent::startup() {
	shader.load_from_files(shader_path("transparent/transparent.vert"), shader_path("transparent/transparent.frag"));
}

void MaterialTransparent::bind_resources(RenderScene &scene) {
	shader.use();
}

void MaterialTransparent::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialTransparent::bind_object_resources(RenderScene &scene, TransparentObject &object) {
	static Texture t;
	static int textured = 0;
	static glm::mat4 view;
	static glm::vec2 window_size;

	auto &rm = ResourceManager::get();

	window_size = scene.full_render_extent; //DisplayManager::get().get_window_size();

	view = scene.view;

	if (object.type == TransparentType::TEXT) {
		t = FontManager::get().fonts[object.texture_name].texture;
		shader.set_int("is_sprite", 0);

	} else if (object.type == TransparentType::SPRITE) {
		t = rm.get_texture(rm.get_texture_handle(object.texture_name));
		shader.set_int("is_sprite", 1);
	}
	textured = !object.texture_name.empty();

	shader.set_mat4("view", view);
	if (object.vertices[0].is_screen_space) {
		shader.set_mat4("projection", glm::ortho(0.0f, window_size.x, 0.0f, window_size.y, 0.1f, 100.0f));
	} else {
		shader.set_mat4("projection", scene.projection);
	}

	shader.set_int("_texture", 0);

	shader.set_int("textured", textured);
	if (object.billboard) {
		auto right = glm::vec3(view[0][0], view[1][0], view[2][0]);
		auto up = glm::vec3(view[0][1], view[1][1], view[2][1]);
		auto look = glm::vec3(view[0][2], view[1][2], view[2][2]);
		shader.set_vec3("camera_right", right);
		shader.set_vec3("camera_up", up);
		shader.set_vec3("camera_look", look);
		shader.set_vec3("camera_pos", scene.camera_pos);
		shader.set_vec2("size", object.size);
		shader.set_vec3("billboard_center", object.position);
		shader.set_int("is_billboard", 1);
		shader.set_int("use_camera_right", (int)object.use_camera_right);
		shader.set_float("billboard_z_offset", object.billboard_z_offset);
	} else {
		shader.set_int("is_billboard", 0);
	}
	shader.set_float("alpha", object.alpha);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, t.id);
}

void MaterialCombination::startup() {
	shader.load_from_files(shader_path("pbr.vert"), shader_path("combination.frag"));
}

void MaterialCombination::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_int("Albedo", 0);
	shader.set_int("Diffuse", 1);
	shader.set_int("Specular", 2);
	shader.set_int("SSAO", 3);
	shader.set_int("AoRoughMetal", 4);
	shader.set_int("ViewPos", 5);
	shader.set_int("Skybox", 6);
	shader.set_int("Particles", 7);
	shader.set_int("Highlights", 8);
	shader.set_int("HighlightsDepth", 9);
	shader.set_int("Depth", 10);

	shader.set_int("use_ao", cvar_use_ao.get());

	shader.set_int("use_fog", cvar_use_fog.get());
	shader.set_float("fog_min", cvar_fog_min.get());
	shader.set_float("fog_max", cvar_fog_max.get());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.albedo_texture_id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, scene.pbr_buffer.diffuse_texture_id);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, scene.pbr_buffer.specular_texture_id);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, scene.ssao_buffer.ssao_texture_id);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.ao_rough_metal_texture_id);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.position_texture_id);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, scene.skybox_buffer.texture_id);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, scene.particle_buffer.texture_id);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, scene.highlight_buffer.texture_id);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, scene.highlight_buffer.depth_texture_id);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.depth_texture_id);
}

void MaterialCombination::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialBloom::startup() {
	shader.load_from_files(shader_path("bloom/bloom.vert"), shader_path("bloom/bloom_combine.frag"));
	downsample.load_from_files(shader_path("bloom/bloom.vert"), shader_path("bloom/downsample.frag"));
	bloom.load_from_files(shader_path("bloom/bloom.vert"), shader_path("bloom/upsample.frag"));
}

void MaterialBloom::bind_resources(RenderScene &scene) {
}

void MaterialBloom::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialShadow::startup() {
	shader.load_from_files(
			shader_path("shadow/shadow.vert"), shader_path("shadow/shadow.frag"), shader_path("shadow/shadow.geom"));
#ifdef WIN32
	skinned_shader.load_from_files(shader_path("shadow/skinned_shadow.vert"), shader_path("shadow/shadow.frag"),
			shader_path("shadow/shadow.geom"));
#endif
}

void MaterialShadow::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_float("far_plane", scene.shadow_buffer.far);
}

void MaterialShadow::bind_instance_resources(ModelInstance &instance, Transform &transform) {
	shader.use();
	shader.set_mat4("model", transform.get_global_model_matrix());
}

void MaterialShadow::bind_light_resources(RenderScene &scene, Light &light, Transform &transform) {
	shader.use();
	shader.set_int("type", (int)light.type);
	if (light.type == LightType::DIRECTIONAL_LIGHT) {
		shader.set_mat4("light_space[0]", scene.shadow_buffer.light_spaces[0]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light.shadow_map_id, 0);
	} else if (light.type == LightType::SPOT_LIGHT) {
		shader.set_mat4("light_space[0]", scene.shadow_buffer.light_spaces[0]);
		shader.set_vec3("light_pos", transform.get_global_position());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light.shadow_map_id, 0);
	} else if (light.type == LightType::POINT_LIGHT) {
		for (int i = 0; i < 6; ++i) {
			shader.set_mat4("light_space[" + std::to_string(i) + "]", scene.shadow_buffer.light_spaces[i]);
		}
		shader.set_vec3("light_pos", transform.get_global_position());
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light.shadow_map_id, 0);
	}

	glClear(GL_DEPTH_BUFFER_BIT);
}

void MaterialShadow::bind_skinned_resources(RenderScene &scene) {
	skinned_shader.use();
	skinned_shader.set_float("far_plane", scene.shadow_buffer.far);
}

void MaterialShadow::bind_instance_resources(SkinnedModelInstance &instance, Transform &transform) {
	skinned_shader.use();
	skinned_shader.set_mat4("model", transform.get_global_model_matrix());
	//TODO: make this functionality in shader function
	glBindBuffer(GL_UNIFORM_BUFFER, instance.skinning_buffer);
	if (!instance.bone_matrices.empty()) {
		glBufferSubData(
				GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * instance.bone_matrices.size(), instance.bone_matrices.data());
	}

	GLuint binding_index = 1;
	GLuint buffer_index = glGetUniformBlockIndex(skinned_shader.id, "SkinningBuffer");
	glUniformBlockBinding(skinned_shader.id, buffer_index, binding_index);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding_index, instance.skinning_buffer);
}

void MaterialShadow::bind_skinned_light_resources(RenderScene &scene, Light &light, Transform &transform) {
	skinned_shader.use();
	skinned_shader.set_int("type", (int)light.type);
	if (light.type == LightType::DIRECTIONAL_LIGHT) {
		skinned_shader.set_mat4("light_space[0]", scene.shadow_buffer.light_spaces[0]);
	} else if (light.type == LightType::SPOT_LIGHT) {
		skinned_shader.set_mat4("light_space[0]", scene.shadow_buffer.light_spaces[0]);
		skinned_shader.set_vec3("light_pos", transform.get_global_position());
	} else if (light.type == LightType::POINT_LIGHT) {
		for (int i = 0; i < 6; ++i) {
			skinned_shader.set_mat4("light_space[" + std::to_string(i) + "]", scene.shadow_buffer.light_spaces[i]);
		}
		skinned_shader.set_vec3("light_pos", transform.get_global_position());
	}
}

void MaterialMousePick::startup() {
	shader.load_from_files(shader_path("gbuffer/gbuffer.vert"), shader_path("mouse_pick.frag"));
#ifdef WIN32
	skinned_shader.load_from_files(shader_path("gbuffer/skinned_gbuffer.vert"), shader_path("mouse_pick.frag"));
#endif
}

void MaterialMousePick::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_mat4("view", scene.view);
	shader.set_mat4("projection", scene.projection);

	skinned_shader.set_mat4("view", scene.view);
	skinned_shader.set_mat4("projection", scene.projection);
}

void MaterialMousePick::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialMousePick::bind_instance_resources(ModelInstance &instance, Transform &transform, Entity entity) {
	shader.set_uint("entity_id", (uint32_t)entity);
	shader.set_mat4("model", transform.get_global_model_matrix());
}

void MaterialMousePick::bind_instance_resources(SkinnedModelInstance &instance, Transform &transform, Entity entity) {
	skinned_shader.set_uint("entity_id", (uint32_t)entity);
	skinned_shader.set_mat4("model", transform.get_global_model_matrix());
}

void MaterialParticle::startup() {
	shader.load_from_files(shader_path("transparent/particles.vert"), shader_path("transparent/particles.frag"));
}

void MaterialParticle::bind_resources(RenderScene &scene) {
	shader.use();
	static glm::mat4 view;

	view = scene.view;

	shader.set_mat4("view", view);
	shader.set_mat4("projection", scene.projection);

	auto right = glm::vec3(view[0][0], view[1][0], view[2][0]);
	auto up = glm::vec3(view[0][1], view[1][1], view[2][1]);
	auto look = glm::vec3(view[0][2], view[1][2], view[2][2]);
	shader.set_vec3("camera_pos", scene.camera_pos);
	shader.set_vec3("camera_look", look);
	shader.set_vec3("camera_right", right);
}

void MaterialParticle::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialHighlight::startup() {
	shader.load_from_files(shader_path("gbuffer/gbuffer.vert"), shader_path("gbuffer/highlight.frag"));
#ifdef WIN32
	skinned_shader.load_from_files(shader_path("gbuffer/skinned_gbuffer.vert"), shader_path("gbuffer/highlight.frag"));
#endif
}

void MaterialHighlight::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_mat4("view", scene.view);
	shader.set_mat4("projection", scene.projection);
	shader.set_vec3("cam_pos", scene.camera_pos);
}

void MaterialHighlight::bind_instance_resources(ModelInstance &instance, Transform &transform) {
	shader.set_mat4("model", transform.get_global_model_matrix());
}

void MaterialHighlight::bind_skinned_resources(RenderScene &scene) {
	skinned_shader.use();
	skinned_shader.set_mat4("view", scene.view);
	skinned_shader.set_mat4("projection", scene.projection);
	skinned_shader.set_vec3("cam_pos", scene.camera_pos);
}

void MaterialHighlight::bind_mesh_resources(Mesh &mesh, HighlightData &highlight_data) {
	shader.set_int("is_highlighted", highlight_data.highlighted);
	shader.set_vec4("highlight_color", glm::vec4(highlight_data.highlight_color, highlight_data.highlight_power));
}

void MaterialHighlight::bind_instance_resources(SkinnedModelInstance &instance, Transform &transform) {
	skinned_shader.set_mat4("model", transform.get_global_model_matrix());

	glBindBuffer(GL_UNIFORM_BUFFER, instance.skinning_buffer);
	if (!instance.bone_matrices.empty()) {
		glBufferSubData(
				GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * instance.bone_matrices.size(), instance.bone_matrices.data());
	}

	GLuint binding_index = 1;
	GLuint buffer_index = glGetUniformBlockIndex(skinned_shader.id, "SkinningBuffer");
	glUniformBlockBinding(skinned_shader.id, buffer_index, binding_index);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding_index, instance.skinning_buffer);
}

void MaterialHighlight::bind_mesh_resources(SkinnedMesh &mesh, HighlightData &highlight_data) {
	skinned_shader.set_int("is_highlighted", highlight_data.highlighted);
	skinned_shader.set_vec4(
			"highlight_color", glm::vec4(highlight_data.highlight_color, highlight_data.highlight_power));
}

void MaterialDecal::startup() {
	shader.load_from_files(shader_path("decal/decal.vert"), shader_path("decal/decal.frag"));
}

void MaterialDecal::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_mat4("inv_view", glm::inverse(scene.view));
	shader.set_mat4("projection", scene.projection);
	shader.set_mat4("view", scene.view);

	shader.set_int("source_position", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.position_texture_id);

	shader.set_int("source_normal", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.normal_texture_id);

	shader.set_int("source_ao_rough_metal", 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, scene.g_buffer.ao_rough_metal_texture_id);
}

void MaterialDecal::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialDecal::bind_decal_resources(Decal &decal, Transform &transform) {
	ResourceManager &resource_manager = ResourceManager::get();
	const glm::vec3 &center = transform.get_global_position();
	const glm::vec3 &scale = transform.get_global_scale() * 0.5f;
	const glm::quat &rotation = transform.get_global_orientation();

	const glm::vec3 &local_up = rotation * glm::vec3(0.0f, 0.0f, 1.0f);
	const glm::vec3 &projector_offset = rotation * glm::vec3(0.0f, scale.y, 0.0f);

	const glm::mat4 &decal_view = glm::lookAt(center + projector_offset, center, glm::normalize(local_up));
	const glm::mat4 &decal_projection = glm::ortho(-scale.x, scale.x, -scale.z, scale.z, 0.0f, scale.y * 2.0f);

	const glm::mat4 &decal_view_proj = decal_projection * decal_view;
	shader.set_mat4("decal_view_proj", decal_view_proj);
	shader.set_mat4("decal_inv_view_proj", glm::inverse(decal_view_proj));
	const Texture &albedo = resource_manager.get_texture(decal.albedo);
	glm::vec2 aspect_ratio;
	if (albedo.width > albedo.height) {
		aspect_ratio.x = 1.0f;
		aspect_ratio.y = float(albedo.width) / float(albedo.height);
	} else {
		aspect_ratio.x = float(albedo.height) / float(albedo.width);
		aspect_ratio.y = 1.0f;
	}
	shader.set_vec4("color", decal.color);
	shader.set_vec3("face_normal", glm::normalize(projector_offset));
	shader.set_vec2("aspect_ratio", aspect_ratio);

	shader.set_int("has_normal", decal.has_normal);
	shader.set_int("use_face_normal", decal.use_face_normal);
	shader.set_int("has_ao", decal.has_ao);
	shader.set_int("has_roughness", decal.has_roughness);
	shader.set_int("has_metalness", decal.has_metalness);

	shader.set_int("decal_albedo", 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, albedo.id);
	if (decal.has_normal) {
		const Texture &normal = resource_manager.get_texture(decal.normal);
		shader.set_int("decal_normal", 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, normal.id);
	}
	if (decal.has_ao || decal.has_roughness || decal.has_metalness) {
		const Texture &ao_rough_metal = resource_manager.get_texture(decal.ao_rough_metal);
		shader.set_int("decal_ao_rough_metal", 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, ao_rough_metal.id);
	}
}
