#include "vk_descriptors.h"

#include "render_manager.h"

vk::DescriptorPool create_pool(vk::Device device, const DescriptorAllocator::PoolSizes &pool_sizes, int count,
		vk::DescriptorPoolCreateFlags flags) {
	std::vector<vk::DescriptorPoolSize> sizes;
	sizes.reserve(pool_sizes.sizes.size());
	for (auto sz : pool_sizes.sizes) {
		sizes.emplace_back(sz.first, uint32_t(sz.second * (float)count));
	}
	vk::DescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = vk::StructureType::eDescriptorPoolCreateInfo;
	pool_info.flags = flags;
	pool_info.maxSets = count;
	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes = sizes.data();

	vk::DescriptorPool descriptor_pool;
	VK_CHECK(device.createDescriptorPool(&pool_info, nullptr, &descriptor_pool));

	return descriptor_pool;
}

void DescriptorAllocator::reset_pools() {
	//reset all used pools and add them to the free pools
	for (auto p : used_pools) {
		device.resetDescriptorPool(p, {});
		free_pools.push_back(p);
	}

	//clear the used pools, since we've put them all in the free pools
	used_pools.clear();

	//reset the current pool handle back to null
	current_pool = VK_NULL_HANDLE;
}

bool DescriptorAllocator::allocate(vk::DescriptorSet *set, vk::DescriptorSetLayout layout) {
	//initialize the current_pool handle if it's null
	if (VK_NULL_HANDLE == current_pool) {
		current_pool = grab_pool();
		used_pools.push_back(current_pool);
	}

	vk::DescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = vk::StructureType::eDescriptorSetAllocateInfo;
	alloc_info.pNext = nullptr;

	alloc_info.pSetLayouts = &layout;
	alloc_info.descriptorPool = current_pool;
	alloc_info.descriptorSetCount = 1;

	//try to allocate the descriptor set
	vk::Result alloc_result = device.allocateDescriptorSets(&alloc_info, set);

	switch (alloc_result) {
		case vk::Result::eSuccess:
			//all good, return
			return true;
		case vk::Result::eErrorFragmentedPool:
		case vk::Result::eErrorOutOfPoolMemory:
			//reallocate pool
			break;
		default:
			//unrecoverable error
			return false;
	}

	//allocate a new pool and retry
	current_pool = grab_pool();
	used_pools.push_back(current_pool);
	alloc_info.descriptorPool = current_pool;

	alloc_result = device.allocateDescriptorSets(&alloc_info, set);

	//if it still fails then we have big issues
	if (alloc_result == vk::Result::eSuccess) {
		return true;
	}

	return false;
}

void DescriptorAllocator::init(vk::Device new_device) {
	device = new_device;
}

void DescriptorAllocator::cleanup() {
	//delete every pool held
	for (auto p : free_pools) {
		device.destroyDescriptorPool(p);
	}
	for (auto p : used_pools) {
		device.destroyDescriptorPool(p);
	}
}

vk::DescriptorPool DescriptorAllocator::grab_pool() {
	//there are reusable pools availible
	if (!free_pools.empty()) {
		//grab pool from the back of the vector and remove it from there.
		vk::DescriptorPool pool = free_pools.back();
		free_pools.pop_back();
		return pool;
	} else {
		//no pools availible, so create a new one
		return create_pool(device, descriptor_sizes, 1000, {});
	}
}

void DescriptorLayoutCache::init(vk::Device new_device) {
	device = new_device;
}

void DescriptorLayoutCache::cleanup() {
	//delete every descriptor layout held
	for (const auto &pair : layout_cache) {
		device.destroyDescriptorSetLayout(pair.second, nullptr);
	}
}

vk::DescriptorSetLayout DescriptorLayoutCache::create_descriptor_layout(vk::DescriptorSetLayoutCreateInfo *info) {
	DescriptorLayoutInfo layout_info;
	layout_info.bindings.reserve(info->bindingCount);
	bool is_sorted = true;
	int last_binding = -1;

	//copy from the direct info struct into our own one
	for (int i = 0; i < info->bindingCount; i++) {
		layout_info.bindings.push_back(info->pBindings[i]);

		//check that the bindings are in strict increasing order
		if (info->pBindings[i].binding > last_binding) {
			last_binding = (int)info->pBindings[i].binding;
		} else {
			is_sorted = false;
		}
	}
	//sort the bindings if they aren't in order
	if (!is_sorted) {
		std::sort(layout_info.bindings.begin(), layout_info.bindings.end(),
				[](vk::DescriptorSetLayoutBinding &a, vk::DescriptorSetLayoutBinding &b) {
					return a.binding < b.binding;
				});
	}

	//try to grab from cache
	auto it = layout_cache.find(layout_info);
	if (it != layout_cache.end()) {
		return (*it).second;
	} else {
		//create a new one (not found)
		vk::DescriptorSetLayout layout;
		VK_CHECK(device.createDescriptorSetLayout(info, nullptr, &layout));

		//add to cache
		layout_cache[layout_info] = layout;
		return layout;
	}
}

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(
		const DescriptorLayoutCache::DescriptorLayoutInfo &other) const {
	if (other.bindings.size() != bindings.size()) {
		return false;
	} else {
		//compare each of the bindings is the same. Bindings are sorted so they will match
		for (int i = 0; i < bindings.size(); i++) {
			if (other.bindings[i].binding != bindings[i].binding) {
				return false;
			}
			if (other.bindings[i].descriptorType != bindings[i].descriptorType) {
				return false;
			}
			if (other.bindings[i].descriptorCount != bindings[i].descriptorCount) {
				return false;
			}
			if (other.bindings[i].stageFlags != bindings[i].stageFlags) {
				return false;
			}
		}
		return true;
	}
}
size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const {
	using std::hash;
	using std::size_t;

	size_t result = hash<size_t>()(bindings.size());

	for (const VkDescriptorSetLayoutBinding &b : bindings) {
		//pack the binding data into a single int64. Not fully correct but it's ok
		size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

		//shuffle the packed binding data and xor it with the main hash
		result ^= hash<size_t>()(binding_hash);
	}

	return result;
}

DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache *layout_cache, DescriptorAllocator *allocator) {
	DescriptorBuilder builder;

	builder.cache = layout_cache;
	builder.alloc = allocator;
	return builder;
}

DescriptorBuilder &DescriptorBuilder::bind_buffer(uint32_t binding, vk::DescriptorBufferInfo *buffer_info,
		vk::DescriptorType type, vk::ShaderStageFlags stage_flags) {
	//create the descriptor binding for the layout
	vk::DescriptorSetLayoutBinding new_binding{};

	new_binding.descriptorCount = 1;
	new_binding.descriptorType = type;
	new_binding.pImmutableSamplers = nullptr;
	new_binding.stageFlags = stage_flags;
	new_binding.binding = binding;

	bindings.push_back(new_binding);

	//create the descriptor write
	vk::WriteDescriptorSet new_write{};
	new_write.sType = vk::StructureType::eWriteDescriptorSet;
	new_write.pNext = nullptr;

	new_write.descriptorCount = 1;
	new_write.descriptorType = type;
	new_write.pBufferInfo = buffer_info;
	new_write.dstBinding = binding;

	writes.push_back(new_write);
	return *this;
}

DescriptorBuilder &DescriptorBuilder::bind_image(uint32_t binding, vk::DescriptorImageInfo *image_info,
		vk::DescriptorType type, vk::ShaderStageFlags stage_flags) {
	//create the descriptor binding for the layout
	vk::DescriptorSetLayoutBinding new_binding{};

	new_binding.descriptorCount = 1;
	new_binding.descriptorType = type;
	new_binding.pImmutableSamplers = nullptr;
	new_binding.stageFlags = stage_flags;
	new_binding.binding = binding;

	bindings.push_back(new_binding);

	//create the descriptor write
	vk::WriteDescriptorSet new_write{};
	new_write.sType = vk::StructureType::eWriteDescriptorSet;
	new_write.pNext = nullptr;

	new_write.descriptorCount = 1;
	new_write.descriptorType = type;
	new_write.pImageInfo = image_info;
	new_write.dstBinding = binding;

	writes.push_back(new_write);
	return *this;
}

bool DescriptorBuilder::build(vk::DescriptorSet &set, vk::DescriptorSetLayout &layout) {
	//build layout first
	vk::DescriptorSetLayoutCreateInfo layout_info{};
	layout_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
	layout_info.pNext = nullptr;

	layout_info.pBindings = bindings.data();
	layout_info.bindingCount = bindings.size();

	layout = cache->create_descriptor_layout(&layout_info);

	//allocate descriptor
	bool success = alloc->allocate(&set, layout);
	if (!success) {
		return false;
	}

	//write descriptor
	for (vk::WriteDescriptorSet &w : writes) {
		w.dstSet = set;
	}

	alloc->device.updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);

	return true;
}

bool DescriptorBuilder::build(vk::DescriptorSet &set) {
	vk::DescriptorSetLayout layout;
	return build(set, layout);
}
