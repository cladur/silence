#include "skybox.h"

#include "display/display_manager.h"
#include "shader.h"
#include "utils.h"

void Skybox::startup() { // Generate cube mesh
}

void Skybox::draw() const {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_map.id);
	utils::render_cube();
}

void Skybox::load_from_directory(const std::string &path) {
	skybox_map.load_from_asset(path + "/environment_map.ktx2");
	irradiance_map.load_from_asset(path + "/irradiance_map.ktx2");
	prefilter_map.load_from_asset(path + "/prefilter_map.ktx2", true);
	brdf_lut.load_from_asset(path + "/brdf_lut.ktx2");

	Shader brdf_shader = {};
	brdf_shader.load_from_files(shader_path("brdf.vert"), shader_path("brdf.frag"));

	// pbr: generate a 2D LUT from the BRDF equations used.
	// ----------------------------------------------------
	glGenTextures(1, &brdf_lut_texture);

	// pre-allocate enough memory for the LUT texture.
	glBindTexture(GL_TEXTURE_2D, brdf_lut_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, 0);
	// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned int capture_fbo;
	unsigned int capture_rbo;
	glGenFramebuffers(1, &capture_fbo);

	// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
	glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_lut_texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);

	glViewport(0, 0, 512, 512);
	brdf_shader.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	utils::render_quad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glm::vec2 framebuffer_size = DisplayManager::get()->get_framebuffer_size();
	glViewport(0, 0, framebuffer_size.x, framebuffer_size.y);
}