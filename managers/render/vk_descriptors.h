#ifndef SILENCE_VK_DESCRIPTORS_H
#define SILENCE_VK_DESCRIPTORS_H

#define VULKAN_HPP_NO_EXCEPTIONS
#include "vulkan/vulkan.hpp"

namespace vk_util {

class DescriptorAllocator {
public:
	struct PoolSizes {
		std::vector<std::pair<vk::DescriptorType, float>> sizes = { { vk::DescriptorType::eSampler, 0.5f },
			{ vk::DescriptorType::eCombinedImageSampler, 4.f }, { vk::DescriptorType::eSampledImage, 4.f },
			{ vk::DescriptorType::eStorageImage, 1.f }, { vk::DescriptorType::eUniformTexelBuffer, 1.f },
			{ vk::DescriptorType::eStorageTexelBuffer, 1.f }, { vk::DescriptorType::eUniformBuffer, 2.f },
			{ vk::DescriptorType::eStorageBuffer, 2.f }, { vk::DescriptorType::eUniformBufferDynamic, 1.f },
			{ vk::DescriptorType::eStorageBufferDynamic, 1.f }, { vk::DescriptorType::eInputAttachment, 0.5f } };
	};

	void reset_pools();
	bool allocate(vk::DescriptorSet *set, vk::DescriptorSetLayout layout);

	void init(vk::Device new_device);
	void cleanup();

	vk::Device device;

private:
	vk::DescriptorPool grab_pool();

	vk::DescriptorPool current_pool{ VK_NULL_HANDLE };
	PoolSizes descriptor_sizes;
	std::vector<vk::DescriptorPool> used_pools;
	std::vector<vk::DescriptorPool> free_pools;
};

class DescriptorLayoutCache {
public:
	void init(vk::Device new_device);
	void cleanup();

	vk::DescriptorSetLayout create_descriptor_layout(vk::DescriptorSetLayoutCreateInfo *info);

	struct DescriptorLayoutInfo {
		//good idea to turn this into an inlined array
		std::vector<vk::DescriptorSetLayoutBinding> bindings;

		bool operator==(const DescriptorLayoutInfo &other) const;

		size_t hash() const;
	};

private:
	struct DescriptorLayoutHash {
		std::size_t operator()(const DescriptorLayoutInfo &k) const {
			return k.hash();
		}
	};

	std::unordered_map<DescriptorLayoutInfo, vk::DescriptorSetLayout, DescriptorLayoutHash> layout_cache;
	vk::Device device;
};

class DescriptorBuilder {
public:
	static DescriptorBuilder begin(DescriptorLayoutCache *layout_cache, DescriptorAllocator *allocator);

	DescriptorBuilder &bind_buffer(uint32_t binding, vk::DescriptorBufferInfo *buffer_info, vk::DescriptorType type,
			vk::ShaderStageFlags stage_flags);
	DescriptorBuilder &bind_image(uint32_t binding, vk::DescriptorImageInfo *image_info, vk::DescriptorType type,
			vk::ShaderStageFlags stage_flags);

	bool build(vk::DescriptorSet &set, vk::DescriptorSetLayout &layout);
	bool build(vk::DescriptorSet &set);

private:
	std::vector<vk::WriteDescriptorSet> writes;
	std::vector<vk::DescriptorSetLayoutBinding> bindings;

	DescriptorLayoutCache *cache;
	DescriptorAllocator *alloc;
};

} //namespace vk_util

#endif //SILENCE_VK_DESCRIPTORS_H
