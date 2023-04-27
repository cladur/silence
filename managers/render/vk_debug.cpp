#include "vk_debug.h"

VkDebug::VkDebug() {
	instance = this;
}

void VkDebug::setup_debugging(RenderManager *render_manager) {
	get_instance()->render_manager = render_manager;
	get_instance()->pfn_vk_set_debug_utils_object_name_ext =
			(PFN_vkSetDebugUtilsObjectNameEXT)render_manager->instance.getProcAddr("vkSetDebugUtilsObjectNameEXT");
}

VkDebug *VkDebug::get_instance() {
	std::lock_guard<std::mutex> lock(mutex);
	if (instance == nullptr) {
		if (instance == nullptr) {
			instance = new VkDebug();
		}
	}
	return instance;
}

VkDebug *VkDebug::instance = nullptr;
std::mutex VkDebug::mutex;