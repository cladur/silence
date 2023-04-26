#include "opengl_context.h"

#include <stb_image_write.h>

#include "opengl/texture.h"
#include "opengl/utils.h"

#include "core/camera/camera.h"
Camera camera;

void OpenGLContext::startup() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(640, 480, "Baker", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	if (window == nullptr) {
		SPDLOG_ERROR("Failed to create GLFW window");
		glfwTerminate();
		assert(false);
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		SPDLOG_ERROR("Failed to initialize OpenGL loader!");
		assert(false);
	}
	SPDLOG_INFO("Successfully initialized OpenGL loader!");

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	irradiance_shader.load_from_files(shader_path("cubemap.vert"), shader_path("irradiance_convulution.frag"));
	prefilter_shader.load_from_files(shader_path("cubemap.vert"), shader_path("prefilter.frag"));
	brdf_shader.load_from_files(shader_path("brdf.vert"), shader_path("brdf.frag"));
}

void OpenGLContext::process_cubemap(const std::string &path, const std::string &export_dir) {
	Texture env_cubemap = {};
	env_cubemap.load_from_asset(path);

	// then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap.id);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
	// ----------------------------------------------------------------------------------------------
	glm::mat4 capture_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 capture_views[] = { glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
										  glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) };

	// pbr: setup framebuffer
	// ----------------------
	unsigned int capture_fbo;
	unsigned int capture_rbo;
	glGenFramebuffers(1, &capture_fbo);
	glGenRenderbuffers(1, &capture_rbo);

	glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);

	// pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
	// --------------------------------------------------------------------------------
	unsigned int irradiance_map;
	glGenTextures(1, &irradiance_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
	// -----------------------------------------------------------------------------
	irradiance_shader.use();
	irradiance_shader.set_int("environment_map", 0);
	irradiance_shader.set_mat4("projection", capture_projection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap.id);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
	for (unsigned int i = 0; i < 6; ++i) {
		irradiance_shader.set_mat4("view", capture_views[i]);
		glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_map, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		utils::render_cube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::string file_names[] = { "px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png" };

	// Save irradiance map to char buffer
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map);
	for (unsigned int face = 0; face < 6; ++face) {
		unsigned char irradiance_map_buffer[32 * 32 * 3];
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, GL_UNSIGNED_BYTE, irradiance_map_buffer);
		// Save irradiance map to png
		stbi_write_png((export_dir + "/" + file_names[face]).c_str(), 32, 32, 3, irradiance_map_buffer, 0);
	}

	// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
	// --------------------------------------------------------------------------------
	unsigned int prefilter_map;
	glGenTextures(1, &prefilter_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	// ----------------------------------------------------------------------------------------------------
	prefilter_shader.use();
	prefilter_shader.set_int("environment_map", 0);
	prefilter_shader.set_mat4("projection", capture_projection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap.id);

	glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
	unsigned int max_mip_levels = 5;
	for (unsigned int mip = 0; mip < max_mip_levels; ++mip) {
		// reisze framebuffer according to mip-level size.
		unsigned int mip_width = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		unsigned int mip_height = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_width, mip_height);
		glViewport(0, 0, mip_width, mip_height);

		float roughness = (float)mip / (float)(max_mip_levels - 1);
		prefilter_shader.set_float("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i) {
			prefilter_shader.set_mat4("view", capture_views[i]);
			glFramebufferTexture2D(
					GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_map, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			utils::render_cube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Save prefilter map to char buffer
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map);
	for (unsigned int mip = 0; mip < max_mip_levels; ++mip) {
		for (unsigned int face = 0; face < 6; ++face) {
			unsigned int mip_width = 128 * std::pow(0.5, mip);
			unsigned int mip_height = 128 * std::pow(0.5, mip);
			unsigned char prefilter_map_buffer[mip_width * mip_height * 3];
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, GL_RGB, GL_UNSIGNED_BYTE, prefilter_map_buffer);
			// Save irradiance map to png
			stbi_write_png((export_dir + "/mip_" + std::to_string(mip) + "_" + file_names[face]).c_str(), mip_width,
					mip_height, 3, prefilter_map_buffer, 0);
		}
	}

	// pbr: generate a 2D LUT from the BRDF equations used.
	// ----------------------------------------------------
	unsigned int brdf_lut_texture;
	glGenTextures(1, &brdf_lut_texture);

	// pre-allocate enough memory for the LUT texture.
	glBindTexture(GL_TEXTURE_2D, brdf_lut_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, 0);
	// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
	glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_lut_texture, 0);

	glViewport(0, 0, 512, 512);
	brdf_shader.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	utils::render_quad();

	// Save brdf lut to char buffer
	unsigned char brdf_lut_buffer[512 * 512 * 3];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, brdf_lut_buffer);
	stbi_write_png((export_dir + "/brdf_lut.png").c_str(), 512, 512, 3, brdf_lut_buffer, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
