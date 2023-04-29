#include "vk_shaders.h"
#include "render/vk_initializers.h"
#include "vk_debug.h"
#include <vulkan/vulkan_core.h>

#include <spirv_reflect.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

bool vk_util::load_shader_module(vk::Device device, const char *file_path, ShaderModule *out_shader_module) {
	std::ifstream file(file_path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return false;
	}

	//find what the size of the file is by looking up the location of the cursor
	//because the cursor is at the end, it gives the size directly in bytes
	size_t file_size = (size_t)file.tellg();

	//spirv expects the buffer to be on uint32, so make sure to reserve an int vector big
	//enough for the entire file
	std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

	//put file cursor at beginning
	file.seekg(0);

	//load the entire file into the buffer
	file.read((char *)buffer.data(), (std::streamsize)file_size);

	//now that the file is loaded into the buffer, we can close it
	file.close();

	//create a new shader module, using the buffer we loaded
	vk::ShaderModuleCreateInfo create_info = {};
	create_info.sType = vk::StructureType::eShaderModuleCreateInfo;
	create_info.pNext = nullptr;

	//codeSize has to be in bytes, so multiply the ints in the buffer by size
	//of int to know the real size of the buffer
	create_info.codeSize = buffer.size() * sizeof(uint32_t);
	create_info.pCode = buffer.data();

	//check that the creation goes well.
	vk::ShaderModule shader_module;
	if (device.createShaderModule(&create_info, nullptr, &shader_module) != vk::Result::eSuccess) {
		return false;
	}

	VkDebug::set_name(shader_module, fmt::format("ShaderModule ({})", file_path).c_str());

	out_shader_module->code = std::move(buffer);
	out_shader_module->module = shader_module;

	RenderManager::get()->main_deletion_queue.push_function([=]() { device.destroyShaderModule(shader_module); });

	return true;
}

// FNV-1a 32bit hashing algorithm.
constexpr uint32_t fnv1a_32(char const *s, std::size_t count) {
	return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
}

uint32_t vk_util::hash_descriptor_layout_info(vk::DescriptorSetLayoutCreateInfo *info) {
	//we are going to put all the data into a string and then hash the string
	std::stringstream ss;

	ss << (VkDescriptorSetLayoutCreateFlags)info->flags;
	ss << info->bindingCount;

	for (auto i = 0u; i < info->bindingCount; i++) {
		const VkDescriptorSetLayoutBinding &binding = info->pBindings[i];

		ss << binding.binding;
		ss << binding.descriptorCount;
		ss << binding.descriptorType;
		ss << binding.stageFlags;
	}

	auto str = ss.str();

	return fnv1a_32(str.c_str(), str.length());
}

void ShaderEffect::ShaderEffect::ShaderEffect::add_stage(ShaderModule *shader_module, vk::ShaderStageFlagBits stage) {
	ShaderStage new_stage = { shader_module, stage };
	stages.push_back(new_stage);
}

struct DescriptorSetLayoutData {
	uint32_t set_number;
	vk::DescriptorSetLayoutCreateInfo create_info;
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
};

void ShaderEffect::ShaderEffect::ShaderEffect::reflect_layout(
		vk::Device device, ReflectionOverrides *overrides, int override_count) {
	std::vector<DescriptorSetLayoutData> set_layouts_data;

	std::vector<vk::PushConstantRange> constant_ranges;

	for (auto &s : stages) {
		SpvReflectShaderModule spv_module;
		SpvReflectResult result = spvReflectCreateShaderModule(
				s.shader_module->code.size() * sizeof(uint32_t), s.shader_module->code.data(), &spv_module);

		uint32_t count = 0;
		result = spvReflectEnumerateDescriptorSets(&spv_module, &count, nullptr);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet *> sets(count);
		result = spvReflectEnumerateDescriptorSets(&spv_module, &count, sets.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		for (auto &set : sets) {
			const SpvReflectDescriptorSet &refl_set = *set;

			DescriptorSetLayoutData layout = {};

			layout.bindings.resize(refl_set.binding_count);
			for (uint32_t i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) {
				const SpvReflectDescriptorBinding &refl_binding = *(refl_set.bindings[i_binding]);
				vk::DescriptorSetLayoutBinding &layout_binding = layout.bindings[i_binding];
				layout_binding.binding = refl_binding.binding;
				layout_binding.descriptorType = static_cast<vk::DescriptorType>(refl_binding.descriptor_type);

				for (int ov = 0; ov < override_count; ov++) {
					if (strcmp(refl_binding.name, overrides[ov].name) == 0) {
						layout_binding.descriptorType = overrides[ov].overriden_type;
					}
				}

				layout_binding.descriptorCount = 1;
				for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
					layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
				}
				layout_binding.stageFlags = static_cast<vk::ShaderStageFlagBits>(spv_module.shader_stage);

				ReflectedBinding reflected{};
				reflected.binding = layout_binding.binding;
				reflected.set = refl_set.set;
				reflected.type = layout_binding.descriptorType;

				bindings[refl_binding.name] = reflected;
			}
			layout.set_number = refl_set.set;
			layout.create_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
			layout.create_info.bindingCount = refl_set.binding_count;
			layout.create_info.pBindings = layout.bindings.data();

			set_layouts_data.push_back(layout);
		}

		//pushconstants

		result = spvReflectEnumeratePushConstantBlocks(&spv_module, &count, nullptr);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectBlockVariable *> pconstants(count);
		result = spvReflectEnumeratePushConstantBlocks(&spv_module, &count, pconstants.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		if (count > 0) {
			vk::PushConstantRange pcs{};
			pcs.offset = pconstants[0]->offset;
			pcs.size = pconstants[0]->size;
			pcs.stageFlags = s.stage;

			constant_ranges.emplace_back(pcs);
		}
	}

	std::array<DescriptorSetLayoutData, 4> merged_layouts;

	for (int i = 0; i < 4; i++) {
		DescriptorSetLayoutData &ly = merged_layouts[i];

		ly.set_number = i;

		ly.create_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;

		std::unordered_map<int, vk::DescriptorSetLayoutBinding> binds;
		for (auto &s : set_layouts_data) {
			if (s.set_number == i) {
				for (auto &b : s.bindings) {
					auto it = binds.find(b.binding);
					if (it == binds.end()) {
						binds[b.binding] = b;
						//ly.bindings.push_back(b);
					} else {
						//merge flags
						binds[b.binding].stageFlags |= b.stageFlags;
					}
				}
			}
		}
		for (auto [k, v] : binds) {
			ly.bindings.emplace_back(v);
		}
		//sort the bindings, for hash purposes
		std::sort(ly.bindings.begin(), ly.bindings.end(),
				[](vk::DescriptorSetLayoutBinding &a, vk::DescriptorSetLayoutBinding &b) {
					return a.binding < b.binding;
				});

		ly.create_info.bindingCount = (uint32_t)ly.bindings.size();
		ly.create_info.pBindings = ly.bindings.data();
		ly.create_info.flags = {};
		ly.create_info.pNext = nullptr;

		if (ly.create_info.bindingCount > 0) {
			set_hashes[i] = vk_util::hash_descriptor_layout_info(&ly.create_info);
			VK_CHECK(device.createDescriptorSetLayout(&ly.create_info, nullptr, &set_layouts[i]));
			VkDebug::set_name(set_layouts[i], fmt::format("Set {} layout", i).c_str());

			RenderManager::get()->main_deletion_queue.push_function(
					[=]() { device.destroyDescriptorSetLayout(set_layouts[i]); });
		} else {
			set_hashes[i] = 0;
			set_layouts[i] = VK_NULL_HANDLE;
		}
	}

	//we start from just the default empty pipeline layout info
	vk::PipelineLayoutCreateInfo mesh_pipeline_layout_info = vk_init::pipeline_layout_create_info();

	mesh_pipeline_layout_info.pPushConstantRanges = constant_ranges.data();
	mesh_pipeline_layout_info.pushConstantRangeCount = (uint32_t)constant_ranges.size();

	std::array<vk::DescriptorSetLayout, 4> compacted_layouts;
	int s = 0;
	for (int i = 0; i < 4; i++) {
		if (set_layouts[i]) {
			compacted_layouts[s] = set_layouts[i];
			s++;
		}
	}

	mesh_pipeline_layout_info.setLayoutCount = s;
	mesh_pipeline_layout_info.pSetLayouts = compacted_layouts.data();

	VK_CHECK(device.createPipelineLayout(&mesh_pipeline_layout_info, nullptr, &built_layout));

	VkDebug::set_name(built_layout, "ShaderEffect Pipeline Layout");

	RenderManager::get()->main_deletion_queue.push_function([=]() { device.destroyPipelineLayout(built_layout); });
}

void ShaderEffect::ShaderEffect::ShaderEffect::fill_stages(
		std::vector<vk::PipelineShaderStageCreateInfo> &pipeline_stages) {
	for (auto &s : stages) {
		pipeline_stages.push_back(vk_init::pipeline_shader_stage_create_info(s.stage, s.shader_module->module));
	}
}

void ShaderDescriptorBinder::bind_buffer(const char *name, const vk::DescriptorBufferInfo &buffer_info) {
	bind_dynamic_buffer(name, -1, buffer_info);
}

void ShaderDescriptorBinder::bind_dynamic_buffer(
		const char *name, uint32_t offset, const vk::DescriptorBufferInfo &buffer_info) {
	auto found = shaders->bindings.find(name);
	if (found != shaders->bindings.end()) {
		const ShaderEffect::ReflectedBinding &bind = (*found).second;

		for (auto &write : buffer_writes) {
			if (write.dst_binding == bind.binding && write.dst_set == bind.set) {
				if (write.buffer_info.buffer != buffer_info.buffer || write.buffer_info.range != buffer_info.range ||
						write.buffer_info.offset != buffer_info.offset) {
					write.buffer_info = buffer_info;
					write.dynamic_offset = offset;

					cached_descriptor_sets[write.dst_set] = VK_NULL_HANDLE;
				} else {
					//already in the write list, but matches buffer
					write.dynamic_offset = offset;
				}

				return;
			}
		}

		BufferWriteDescriptor new_write;
		new_write.dst_set = bind.set;
		new_write.dst_binding = bind.binding;
		new_write.descriptor_type = bind.type;
		new_write.buffer_info = buffer_info;
		new_write.dynamic_offset = offset;

		cached_descriptor_sets[bind.set] = VK_NULL_HANDLE;

		buffer_writes.push_back(new_write);
	}
}

void ShaderDescriptorBinder::apply_binds(vk::CommandBuffer cmd) {
	for (int i = 0; i < 2; i++) {
		//there are writes for this set
		if (cached_descriptor_sets[i]) {
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shaders->built_layout, i, 1,
					&cached_descriptor_sets[i], set_offsets[i].count, set_offsets[i].offsets.data());
		}
	}
}

void ShaderDescriptorBinder::build_sets(vk::Device device, vk_util::DescriptorAllocator &allocator) {
	std::array<std::vector<vk::WriteDescriptorSet>, 4> writes{};

	std::sort(buffer_writes.begin(), buffer_writes.end(), [](BufferWriteDescriptor &a, BufferWriteDescriptor &b) {
		if (b.dst_set == a.dst_set) {
			return a.dst_set < b.dst_set;
		} else {
			return a.dst_binding < b.dst_binding;
		}
	});

	//reset the dynamic offsets
	for (auto &s : set_offsets) {
		s.count = 0;
	}

	for (BufferWriteDescriptor &w : buffer_writes) {
		uint32_t set = w.dst_set;
		vk::WriteDescriptorSet write =
				vk_init::write_descriptor_buffer(w.descriptor_type, VK_NULL_HANDLE, &w.buffer_info, w.dst_binding);

		writes[set].push_back(write);

		//dynamic offsets
		if (w.descriptor_type == vk::DescriptorType::eUniformBufferDynamic ||
				w.descriptor_type == vk::DescriptorType::eStorageBufferDynamic) {
			DynOffsets &offset_set = set_offsets[set];
			offset_set.offsets[offset_set.count] = w.dynamic_offset;
			offset_set.count++;
		}
	}

	for (int i = 0; i < 4; i++) {
		//there are writes for this set
		if (!writes[i].empty()) {
			if (!cached_descriptor_sets[i]) {
				//alloc
				auto layout = shaders->set_layouts[i];

				vk::DescriptorSet new_descriptor;
				allocator.allocate(&new_descriptor, layout);

				for (auto &w : writes[i]) {
					w.dstSet = new_descriptor;
				}
				device.updateDescriptorSets((uint32_t)writes[i].size(), writes[i].data(), 0, nullptr);

				cached_descriptor_sets[i] = new_descriptor;
			}
		}
	}
}

void ShaderDescriptorBinder::set_shader(ShaderEffect *new_shader) {
	//invalidate nonequal layouts
	if (shaders && shaders != new_shader) {
		for (int i = 0; i < 4; i++) {
			if (new_shader->set_hashes[i] != shaders->set_hashes[i]) {
				cached_descriptor_sets[i] = VK_NULL_HANDLE;
			} else if (new_shader->set_hashes[i] == 0) {
				cached_descriptor_sets[i] = VK_NULL_HANDLE;
			}
		}
	} else {
		for (int i = 0; i < 4; i++) {
			cached_descriptor_sets[i] = VK_NULL_HANDLE;
		}
	}

	shaders = new_shader;
}

ShaderModule *ShaderCache::get_shader(const std::string &path) {
	auto it = module_cache.find(path);
	if (it == module_cache.end()) {
		ShaderModule new_shader;

		bool result = vk_util::load_shader_module(device, path.c_str(), &new_shader);
		if (!result) {
			SPDLOG_ERROR("Error when compiling shader {}", path);
			return nullptr;
		}

		module_cache[path] = new_shader;
	}
	return &module_cache[path];
}
