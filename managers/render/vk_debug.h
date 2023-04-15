#ifndef SILENCE_VK_DEBUG_H
#define SILENCE_VK_DEBUG_H

#include "render_manager.h"
#include <typeindex>
#include <vulkan/vulkan_enums.hpp>

class VkDebug {
private:
	RenderManager *render_manager = nullptr;
	PFN_vkSetDebugUtilsObjectNameEXT pfn_vk_set_debug_utils_object_name_ext = VK_NULL_HANDLE;
	static VkDebug *instance;
	static std::mutex mutex;
	std::map<std::type_index, vk::ObjectType> object_map = {
		{ std::type_index(typeid(VkInstance)), vk::ObjectType::eInstance },
		{ std::type_index(typeid(VkPhysicalDevice)), vk::ObjectType::ePhysicalDevice },
		{ std::type_index(typeid(VkDevice)), vk::ObjectType::eDevice },
		{ std::type_index(typeid(VkQueue)), vk::ObjectType::eQueue },
		{ std::type_index(typeid(VkFramebuffer)), vk::ObjectType::eFramebuffer },
	};

public:
	VkDebug();
	VkDebug(VkDebug &other) = delete;
	void operator=(const VkDebug &) = delete;
	static VkDebug *get_instance();

	static void setup_debugging(RenderManager *render_manager);

	template <typename T> static void set_object_name(T object, const char *object_name) {
		VkDebug *instance = get_instance();
		vk::DebugUtilsObjectNameInfoEXT name_info;
		auto casted_handle = uint64_t(static_cast<T::CType>(object));
		auto object_type = instance->object_map[std::type_index(typeid(T))];

		name_info.sType = vk::StructureType::eDebugUtilsObjectNameInfoEXT;
		name_info.objectType = object_type;
		name_info.objectHandle = casted_handle;
		name_info.pObjectName = object_name;

		instance->pfn_vk_set_debug_utils_object_name_ext(
				instance->render_manager->device, reinterpret_cast<const VkDebugUtilsObjectNameInfoEXT *>(&name_info));
	}
}; //namespace vk_debug

#endif //SILENCE_VK_DEBUG_H
