#include "skybox.h"

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
	prefilter_map.load_from_asset(path + "/prefilter_map.ktx2");
	brdf_lut.load_from_asset(path + "/brdf_lut.ktx2");
}