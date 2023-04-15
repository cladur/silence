#ifndef SILENCE_VK_DEBUG_H
#define SILENCE_VK_DEBUG_H

#include "render_manager.h"
#include <vulkan/vulkan_core.h>
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
		{ std::type_index(typeid(VkCommandBuffer)), vk::ObjectType::eCommandBuffer },
		{ std::type_index(typeid(VkImage)), vk::ObjectType::eImage },
		{ std::type_index(typeid(VkSampler)), vk::ObjectType::eSampler },
		{ std::type_index(typeid(VkBuffer)), vk::ObjectType::eBuffer },
		{ std::type_index(typeid(VkDeviceMemory)), vk::ObjectType::eDeviceMemory },
		{ std::type_index(typeid(VkShaderModule)), vk::ObjectType::eShaderModule },
		{ std::type_index(typeid(VkPipeline)), vk::ObjectType::ePipeline },
		{ std::type_index(typeid(VkPipelineLayout)), vk::ObjectType::ePipelineLayout },
		{ std::type_index(typeid(VkRenderPass)), vk::ObjectType::eRenderPass },
		{ std::type_index(typeid(VkDescriptorSet)), vk::ObjectType::eDescriptorSet },
		{ std::type_index(typeid(VkDescriptorSetLayout)), vk::ObjectType::eDescriptorSetLayout },
		{ std::type_index(typeid(VkSemaphore)), vk::ObjectType::eSemaphore },
		{ std::type_index(typeid(VkFence)), vk::ObjectType::eFence },
		{ std::type_index(typeid(VkEvent)), vk::ObjectType::eEvent },
	};

public:
	VkDebug();
	VkDebug(VkDebug &other) = delete;
	void operator=(const VkDebug &) = delete;
	static VkDebug *get_instance();

	static void setup_debugging(RenderManager *render_manager);

	template <typename T> static void set_name(T object, const char *object_name) {
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
