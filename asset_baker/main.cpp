#include "animation_asset.h"
#include "asset_loader.h"
#include "material_asset.h"
#include "mesh_asset.h"
#include "model_asset.h"
#include "skinned_mesh_asset.h"
#include "skinned_model_asset.h"
#include "texture_asset.h"
#include <ktx.h>
#include <filesystem>
#include <fstream>
#include <future>
#include <glm/ext/matrix_transform.hpp>
#include <string>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

#include "opengl_context.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace tg = tinygltf;
namespace fs = std::filesystem;
using namespace assets;

size_t find_case_insensitive(std::string data, std::string to_search, size_t pos = 0) {
	// Convert complete given String to lower case
	std::transform(data.begin(), data.end(), data.begin(), ::tolower);
	// Convert complete given Sub String to lower case
	std::transform(to_search.begin(), to_search.end(), to_search.begin(), ::tolower);
	// Find sub string in given string
	return data.find(to_search, pos);
}

struct ConverterState {
	fs::path asset_path;
	fs::path export_path;

	[[nodiscard]] fs::path convert_to_export_relative(const fs::path &path) const;
};

fs::path ConverterState::convert_to_export_relative(const fs::path &path) const {
	return path.lexically_proximate(export_path);
}

bool convert_image(const fs::path &input, const fs::path &output) {
	// Check if input filename has "normal" in it, case-insensitive
	auto normal_pos = find_case_insensitive(input.string(), "normal");
	auto roughness_pos = find_case_insensitive(input.string(), "roughness");
	auto metallic_pos = find_case_insensitive(input.string(), "metal");

	// TODO: Add optional flag for using UASTC instead of ETC1S for higher quality + more quality options
	// https://github.com/KhronosGroup/3D-Formats-Guidelines/blob/main/KTXArtistGuide.md#compression-examples

	std::string params;
	std::string hq_params;

	if (normal_pos || roughness_pos || metallic_pos) {
		params = "--encode uastc --uastc_quality 0 --uastc_rdo_l 2.0 --zcmp 16";
		hq_params = "--encode uastc --uastc_quality 3 --uastc_rdo_l .5 --uastc_rdo_d 32768 --zcmp 16";
	} else {
		params = "--encode etc1s --clevel 4 --qlevel 255";
		hq_params = "--encode uastc --uastc_quality 2 --uastc_rdo_l 1.0 --uastc_rdo_d 32768 --zcmp 16";
	}

	auto result =
			system(fmt::format("toktx --t2 --genmipmap {} {} {}", params, output.string(), input.string()).c_str());

	SPDLOG_INFO("Converted image {} ", input.string());

	return result == 0;
}

void pack_vertex(
		assets::VertexPNV32 &new_vert, float vx, float vy, float vz, float nx, float ny, float nz, float ux, float uy) {
	new_vert.position[0] = vx;
	new_vert.position[1] = vy;
	new_vert.position[2] = vz;

	new_vert.normal[0] = nx;
	new_vert.normal[1] = ny;
	new_vert.normal[2] = nz;

	new_vert.uv[0] = ux;
	new_vert.uv[1] = 1 - uy;
}

void pack_vertex(assets::VertexP32N8C8V16 &new_vert, float vx, float vy, float vz, float nx, float ny, float nz,
		float ux, float uy) {
	new_vert.position[0] = vx;
	new_vert.position[1] = vy;
	new_vert.position[2] = vz;

	new_vert.normal[0] = uint8_t(((nx + 1.0) / 2.0) * 255);
	new_vert.normal[1] = uint8_t(((ny + 1.0) / 2.0) * 255);
	new_vert.normal[2] = uint8_t(((nz + 1.0) / 2.0) * 255);

	new_vert.uv[0] = ux;
	new_vert.uv[1] = 1 - uy;
}

std::string calculate_gltf_mesh_name(tg::Model &model, int mesh_index, int primitive_index) {
	auto mesh_index_string = std::to_string(mesh_index);
	auto prim_index_string = std::to_string(primitive_index);

	std::string meshname = "MESH_" + mesh_index_string + "_" + model.meshes[mesh_index].name;

	bool multiprim = model.meshes[mesh_index].primitives.size() > 1;
	if (multiprim) {
		meshname += "_PRIM_" + prim_index_string;
	}

	return meshname;
}

void unpack_gltf_buffer(tg::Model &model, tg::Accessor &accessor, std::vector<uint8_t> &output_buffer) {
	int buffer_id = accessor.bufferView;
	size_t element_size = tg::GetComponentSizeInBytes(accessor.componentType);

	tg::BufferView &buffer_view = model.bufferViews[buffer_id];

	tg::Buffer &buffer_data = (model.buffers[buffer_view.buffer]);

	uint8_t *data_ptr = buffer_data.data.data() + accessor.byteOffset + buffer_view.byteOffset;

	int components = tg::GetNumComponentsInType(accessor.type);

	element_size *= components;

	size_t stride = buffer_view.byteStride;
	if (stride == 0) {
		stride = element_size;
	}

	output_buffer.resize(accessor.count * element_size);

	for (int i = 0; i < accessor.count; i++) {
		uint8_t *dataindex = data_ptr + stride * i;
		uint8_t *targetptr = output_buffer.data() + element_size * i;

		memcpy(targetptr, dataindex, element_size);
	}
}

void extract_gltf_vertices(tg::Primitive &primitive, tg::Model &model, std::vector<assets::VertexPNV32> &vertices) {
	tg::Accessor &pos_accesor = model.accessors[primitive.attributes["POSITION"]];

	vertices.resize(pos_accesor.count);

	std::vector<uint8_t> pos_data;
	unpack_gltf_buffer(model, pos_accesor, pos_data);

	for (int i = 0; i < vertices.size(); i++) {
		if (pos_accesor.type == TINYGLTF_TYPE_VEC3) {
			if (pos_accesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				auto *dtf = (float *)pos_data.data();

				//vec3f
				vertices[i].position[0] = *(dtf + (i * 3) + 0);
				vertices[i].position[1] = *(dtf + (i * 3) + 1);
				vertices[i].position[2] = *(dtf + (i * 3) + 2);
			} else {
				assert(false);
			}
		} else {
			assert(false);
		}
	}

	tg::Accessor &normal_accesor = model.accessors[primitive.attributes["NORMAL"]];

	std::vector<uint8_t> normal_data;
	unpack_gltf_buffer(model, normal_accesor, normal_data);

	for (int i = 0; i < vertices.size(); i++) {
		if (normal_accesor.type == TINYGLTF_TYPE_VEC3) {
			if (normal_accesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				auto *dtf = (float *)normal_data.data();

				//vec3f
				vertices[i].normal[0] = *(dtf + (i * 3) + 0);
				vertices[i].normal[1] = *(dtf + (i * 3) + 1);
				vertices[i].normal[2] = *(dtf + (i * 3) + 2);
			} else {
				assert(false);
			}
		} else {
			assert(false);
		}
	}

	tg::Accessor &uv_accesor = model.accessors[primitive.attributes["TEXCOORD_0"]];

	std::vector<uint8_t> uv_data;
	unpack_gltf_buffer(model, uv_accesor, uv_data);

	for (int i = 0; i < vertices.size(); i++) {
		if (uv_accesor.type == TINYGLTF_TYPE_VEC2) {
			if (uv_accesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				auto *dtf = (float *)uv_data.data();

				//vec3f
				vertices[i].uv[0] = *(dtf + (i * 2) + 0);
				vertices[i].uv[1] = *(dtf + (i * 2) + 1);
			} else {
				assert(false);
			}
		} else {
			assert(false);
		}
	}
}

void extract_gltf_skinned_vertices(
		tg::Primitive &primitive, tg::Model &model, std::vector<assets::SkinnedVertexPNV32> &vertices) {
	tg::Accessor &pos_accesor = model.accessors[primitive.attributes["POSITION"]];

	vertices.resize(pos_accesor.count);

	std::vector<uint8_t> pos_data;
	unpack_gltf_buffer(model, pos_accesor, pos_data);

	for (int i = 0; i < vertices.size(); i++) {
		if (pos_accesor.type == TINYGLTF_TYPE_VEC3 && pos_accesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
			auto *dtf = (float *)pos_data.data();

			//vec3f
			vertices[i].position[0] = *(dtf + (i * 3) + 0);
			vertices[i].position[1] = *(dtf + (i * 3) + 1);
			vertices[i].position[2] = *(dtf + (i * 3) + 2);
		} else {
			SPDLOG_ERROR("INVALID POSITION TYPE");
			assert(false);
		}
	}

	tg::Accessor &normal_accesor = model.accessors[primitive.attributes["NORMAL"]];

	std::vector<uint8_t> normal_data;
	unpack_gltf_buffer(model, normal_accesor, normal_data);

	for (int i = 0; i < vertices.size(); i++) {
		if (normal_accesor.type == TINYGLTF_TYPE_VEC3 &&
				normal_accesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
			auto *dtf = (float *)normal_data.data();

			//vec3f
			vertices[i].normal[0] = *(dtf + (i * 3) + 0);
			vertices[i].normal[1] = *(dtf + (i * 3) + 1);
			vertices[i].normal[2] = *(dtf + (i * 3) + 2);
		} else {
			SPDLOG_ERROR("INVALID NORMAL TYPE");
			assert(false);
		}
	}

	tg::Accessor &uv_accesor = model.accessors[primitive.attributes["TEXCOORD_0"]];

	std::vector<uint8_t> uv_data;
	unpack_gltf_buffer(model, uv_accesor, uv_data);

	for (int i = 0; i < vertices.size(); i++) {
		if (uv_accesor.type == TINYGLTF_TYPE_VEC2 && uv_accesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
			auto *dtf = (float *)uv_data.data();

			//vec3f
			vertices[i].uv[0] = *(dtf + (i * 2) + 0);
			vertices[i].uv[1] = *(dtf + (i * 2) + 1);
		} else {
			SPDLOG_ERROR("INVALID UV TYPE");
			assert(false);
		}
	}

	tg::Accessor &joint_accessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];

	std::vector<uint8_t> joint_data;
	unpack_gltf_buffer(model, joint_accessor, joint_data);

	for (int i = 0; i < vertices.size(); i++) {
		if (joint_accessor.type == TINYGLTF_TYPE_VEC4 &&
				joint_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
			auto *dtf = (uint8_t *)joint_data.data();

			vertices[i].joint[0] = *(dtf + (i * 4) + 0);
			vertices[i].joint[1] = *(dtf + (i * 4) + 1);
			vertices[i].joint[2] = *(dtf + (i * 4) + 2);
			vertices[i].joint[3] = *(dtf + (i * 4) + 3);
		} else if (joint_accessor.type == TINYGLTF_TYPE_VEC4 &&
				joint_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
			auto *dtf = (uint16_t *)joint_data.data();

			vertices[i].joint[0] = *(dtf + (i * 4) + 0);
			vertices[i].joint[1] = *(dtf + (i * 4) + 1);
			vertices[i].joint[2] = *(dtf + (i * 4) + 2);
			vertices[i].joint[3] = *(dtf + (i * 4) + 3);
		} else {
			SPDLOG_ERROR("INVALID JOINT TYPE");
			assert(false);
		}
	}

	tg::Accessor &weight_accessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];

	std::vector<uint8_t> weight_data;
	unpack_gltf_buffer(model, weight_accessor, weight_data);

	for (int i = 0; i < vertices.size(); i++) {
		if (weight_accessor.type == TINYGLTF_TYPE_VEC4 &&
				weight_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
			auto *dtf = (float *)weight_data.data();

			//vec3f
			vertices[i].weight[0] = *(dtf + (i * 4) + 0);
			vertices[i].weight[1] = *(dtf + (i * 4) + 1);
			vertices[i].weight[2] = *(dtf + (i * 4) + 2);
			vertices[i].weight[3] = *(dtf + (i * 4) + 3);
		} else {
			SPDLOG_ERROR("INVALID WEIGHT TYPE");
			assert(false);
		}
	}
}

void extract_gltf_animation_nodes(
		tg::AnimationChannel &channel, tg::Animation &animation, tg::Model &model, assets::NodeAnimation &node) {
	const tinygltf::AnimationSampler &sampler = animation.samplers[channel.sampler];

	tg::Accessor &times_accessor = model.accessors[sampler.input];

	std::vector<uint8_t> times_data;
	unpack_gltf_buffer(model, times_accessor, times_data);

	tg::Accessor &transform_accessor = model.accessors[sampler.output];

	std::vector<uint8_t> transform_data;
	unpack_gltf_buffer(model, transform_accessor, transform_data);

	const int32_t node_index = channel.target_node;
	std::string node_name = model.nodes[node_index].name;
	constexpr int32_t SECONDS_TO_MS = 1000;
	if (transform_accessor.type == TINYGLTF_TYPE_VEC3 &&
			transform_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
		auto *times = (float *)times_data.data();
		auto *transforms = (float *)transform_data.data();
		node.translation_times.reserve(times_accessor.count);
		node.translations.reserve(times_accessor.count);

		for (size_t i = 0; i < times_accessor.count; ++i) {
			node.translation_times.push_back(times[i] * SECONDS_TO_MS);
			std::array<float, 3> translation = { transforms[i * 3], transforms[i * 3 + 1], transforms[i * 3 + 2] };
			node.translations.emplace_back(translation);
		}
	} else if (transform_accessor.type == TINYGLTF_TYPE_VEC4 &&
			transform_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
		auto *times = (float *)times_data.data();
		auto *transforms = (float *)transform_data.data();
		node.rotation_times.reserve(times_accessor.count);
		node.rotations.reserve(times_accessor.count);

		for (size_t i = 0; i < times_accessor.count; ++i) {
			node.rotation_times.push_back(times[i] * SECONDS_TO_MS);
			std::array<float, 4> translation = { transforms[i * 4], transforms[i * 4 + 1], transforms[i * 4 + 2],
				transforms[i * 4 + 3] };
			node.rotations.emplace_back(translation);
		}
	} else {
		SPDLOG_WARN("Invalid transform type at node: {}", node_name);
	}
}

void extract_gltf_indices(tg::Primitive &primitive, tg::Model &model, std::vector<uint32_t> &prim_indices) {
	int index_accesor = primitive.indices;

	int index_buffer = model.accessors[index_accesor].bufferView;
	int component_type = model.accessors[index_accesor].componentType;
	size_t index_size = tg::GetComponentSizeInBytes(component_type);

	tg::BufferView &indexview = model.bufferViews[index_buffer];
	int bufferidx = indexview.buffer;

	tg::Buffer &buffindex = (model.buffers[bufferidx]);

	uint8_t *data_ptr = buffindex.data.data() + indexview.byteOffset;

	std::vector<uint8_t> unpacked_indices;
	unpack_gltf_buffer(model, model.accessors[index_accesor], unpacked_indices);

	for (int i = 0; i < model.accessors[index_accesor].count; i++) {
		uint32_t index;
		switch (component_type) {
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
				auto *bfr = (uint16_t *)unpacked_indices.data();
				index = *(bfr + i);
			} break;
			case TINYGLTF_COMPONENT_TYPE_SHORT: {
				auto *bfr = (int16_t *)unpacked_indices.data();
				index = *(bfr + i);
			} break;
			default:
				assert(false);
		}

		prim_indices.push_back(index);
	}

	for (int i = 0; i < prim_indices.size() / 3; i++) {
		//flip the triangle
		std::swap(prim_indices[i * 3 + 1], prim_indices[i * 3 + 2]);
	}
}

bool extract_gltf_meshes(
		tg::Model &model, const fs::path &input, const fs::path &output_folder, const ConverterState &conv_state) {
	for (auto mesh_index = 0; mesh_index < model.meshes.size(); mesh_index++) {
		auto &mesh = model.meshes[mesh_index];

		using VertexFormat = assets::VertexPNV32;
		auto vertex_format_enum = assets::VertexFormat::PNV32;

		std::vector<VertexFormat> vertices;
		std::vector<uint32_t> indices;

		for (auto prim_index = 0; prim_index < mesh.primitives.size(); prim_index++) {
			vertices.clear();
			indices.clear();

			std::string mesh_name = calculate_gltf_mesh_name(model, mesh_index, prim_index);

			auto &primitive = mesh.primitives[prim_index];

			extract_gltf_indices(primitive, model, indices);
			extract_gltf_vertices(primitive, model, vertices);

			MeshInfo mesh_info;
			mesh_info.vertex_format = vertex_format_enum;
			mesh_info.vertex_buffer_size = vertices.size() * sizeof(VertexFormat);
			mesh_info.index_buffer_size = indices.size() * sizeof(uint32_t);
			mesh_info.index_size = indices.size();
			mesh_info.original_file = input.string();
			mesh_info.bounds = calculate_bounds(vertices.data(), vertices.size());
			mesh_info.compression_mode = assets::CompressionMode::LZ4;

			AssetFile new_file = pack_mesh(&mesh_info, (char *)vertices.data(), (char *)indices.data());

			fs::path mesh_path = output_folder / (mesh_name + ".mesh");

			save_binary_file(mesh_path.string().c_str(), new_file);
		}
	}

	return true;
}

bool extract_gltf_skinned_meshes(
		tg::Model &model, const fs::path &input, const fs::path &output_folder, const ConverterState &conv_state) {
	for (auto mesh_index = 0; mesh_index < model.meshes.size(); mesh_index++) {
		auto &mesh = model.meshes[mesh_index];

		using SkinnedVertexFormat = assets::SkinnedVertexPNV32;
		auto vertex_format_enum = assets::SkinnedVertexFormat::PNV32;

		std::vector<SkinnedVertexFormat> vertices;
		std::vector<uint32_t> indices;

		for (auto prim_index = 0; prim_index < mesh.primitives.size(); prim_index++) {
			vertices.clear();
			indices.clear();

			std::string mesh_name = calculate_gltf_mesh_name(model, mesh_index, prim_index);

			auto &primitive = mesh.primitives[prim_index];

			extract_gltf_indices(primitive, model, indices);
			extract_gltf_skinned_vertices(primitive, model, vertices);

			SkinnedMeshInfo mesh_info;
			mesh_info.vertex_format = vertex_format_enum;
			mesh_info.vertex_buffer_size = vertices.size() * sizeof(SkinnedVertexFormat);
			mesh_info.index_buffer_size = indices.size() * sizeof(uint32_t);
			mesh_info.index_size = indices.size();
			mesh_info.original_file = input.string();
			mesh_info.bounds = calculate_bounds(vertices.data(), vertices.size());
			mesh_info.compression_mode = assets::CompressionMode::LZ4;

			AssetFile new_file = pack_mesh(&mesh_info, (char *)vertices.data(), (char *)indices.data());

			fs::path mesh_path = output_folder / (mesh_name + ".skinned_mesh");

			save_binary_file(mesh_path.string().c_str(), new_file);
		}
	}

	return true;
}

bool extract_gltf_animations(tg::Model &model, std::string model_name, tg::Animation &animation, const fs::path &input,
		const fs::path &output_folder, const ConverterState &conv_state) {
	assets::AnimationInfo animation_info;
	size_t full_size = 0;
	std::vector<assets::NodeAnimation> nodes(model.nodes.size());
	animation_info.sizes.resize(model.nodes.size());
	animation_info.node_names.resize(model.nodes.size());

	for (tinygltf::AnimationChannel &channel : animation.channels) {
		if (strcmp(channel.target_path.c_str(), "scale") == 0) { //Skip scale
			continue;
		}
		const int32_t node_index = channel.target_node;
		std::string node_name = model.nodes[node_index].name;
		assets::NodeAnimation *node = &nodes[node_index];
		extract_gltf_animation_nodes(channel, animation, model, *node);

		if (strcmp(channel.target_path.c_str(), "translation") == 0) {
			animation_info.sizes[node_index].translations = node->translations.size() * sizeof(glm::vec3);
			animation_info.sizes[node_index].translation_times = node->translation_times.size() * sizeof(float);
			full_size += node->translations.size() * sizeof(glm::vec3) + node->translation_times.size() * sizeof(float);
		} else if (strcmp(channel.target_path.c_str(), "rotation") == 0) {
			animation_info.sizes[node_index].rotations = node->rotations.size() * sizeof(glm::quat);
			animation_info.sizes[node_index].rotation_times = node->rotation_times.size() * sizeof(float);
			full_size += node->rotations.size() * sizeof(glm::quat) + node->rotation_times.size() * sizeof(float);
		}

		animation_info.node_names[node_index] = node_name;
	}

	// TODO: Optimize it
	for (int32_t i = nodes.size() - 1; i >= 0; --i) {
		if (nodes[i].translations.empty() || nodes[i].rotations.empty()) {
			nodes.erase(nodes.begin() + i);
			animation_info.sizes.erase(animation_info.sizes.begin() + i);
			animation_info.node_names.erase(animation_info.node_names.begin() + i);
		}
	}

	if (!model.accessors.empty() && !animation.samplers.empty() &&
			!model.accessors[animation.samplers[0].input].maxValues.empty()) {
		animation_info.duration_seconds = (float)model.accessors[animation.samplers[0].input].maxValues[0];
	} else {
		animation_info.duration_seconds = 0.0f;
	}

	animation_info.original_file = input.string();
	animation_info.full_size = full_size;
	std::string animation_name = model_name + "_" + animation.name;

	AssetFile new_file = assets::pack_animation(&animation_info, nodes);

	fs::path animation_path = output_folder / (animation_name + ".anim");

	save_binary_file(animation_path.string().c_str(), new_file);

	return true;
}

std::string calculate_gltf_material_name(tinygltf::Model &model, int material_index) {
	auto material_index_string = std::to_string(material_index);
	std::string matname = "MAT_" + material_index_string + "_" + model.materials[material_index].name;
	return matname;
}

void extract_gltf_materials(tinygltf::Model &model, const fs::path &input, const fs::path &output_folder,
		const ConverterState &conv_state) {
	int nm = 0;
	for (auto &material : model.materials) {
		std::string matname = calculate_gltf_material_name(model, nm);

		nm++;
		auto &pbr = material.pbrMetallicRoughness;

		MaterialInfo new_material;
		new_material.base_effect = "defaultPBR";

		{
			if (pbr.baseColorTexture.index < 0) {
				pbr.baseColorTexture.index = 0;
			}
			auto base_color = model.textures[pbr.baseColorTexture.index];
			auto base_image = model.images[base_color.source];

			fs::path base_color_path = output_folder.parent_path() / base_image.uri;

			base_color_path.replace_extension(".ktx2");

			base_color_path = conv_state.convert_to_export_relative(base_color_path);

			new_material.textures["baseColor"] = base_color_path.string();
		}
		if (pbr.metallicRoughnessTexture.index >= 0) {
			auto image = model.textures[pbr.metallicRoughnessTexture.index];
			auto base_image = model.images[image.source];

			fs::path base_color_path = output_folder.parent_path() / base_image.uri;

			base_color_path.replace_extension(".ktx2");

			base_color_path = conv_state.convert_to_export_relative(base_color_path);

			new_material.textures["metallicRoughness"] = base_color_path.string();
		}

		if (material.normalTexture.index >= 0) {
			auto image = model.textures[material.normalTexture.index];
			auto base_image = model.images[image.source];

			fs::path base_color_path = output_folder.parent_path() / base_image.uri;

			base_color_path.replace_extension(".ktx2");

			base_color_path = conv_state.convert_to_export_relative(base_color_path);

			new_material.textures["normals"] = base_color_path.string();
		}

		if (material.occlusionTexture.index >= 0) {
			auto image = model.textures[material.occlusionTexture.index];
			auto base_image = model.images[image.source];

			fs::path base_color_path = output_folder.parent_path() / base_image.uri;

			base_color_path.replace_extension(".ktx2");

			base_color_path = conv_state.convert_to_export_relative(base_color_path);

			new_material.textures["occlusion"] = base_color_path.string();
		}

		if (material.emissiveTexture.index >= 0) {
			auto image = model.textures[material.emissiveTexture.index];
			auto base_image = model.images[image.source];

			fs::path base_color_path = output_folder.parent_path() / base_image.uri;

			base_color_path.replace_extension(".ktx2");

			base_color_path = conv_state.convert_to_export_relative(base_color_path);

			new_material.textures["emissive"] = base_color_path.string();
		}

		fs::path material_path = output_folder / (matname + ".mat");

		if (material.alphaMode == "BLEND") {
			new_material.transparency = TransparencyMode::Transparent;
		} else {
			new_material.transparency = TransparencyMode::Opaque;
		}

		assets::AssetFile new_file = assets::pack_material(&new_material);

		//save to disk
		save_binary_file(material_path.string().c_str(), new_file);
	}
}

void extract_gltf_nodes(tinygltf::Model &model, const fs::path &input, const fs::path &output_folder,
		const ConverterState &conv_state) {
	assets::ModelInfo model_info;

	std::vector<uint64_t> mesh_nodes;
	for (int i = 0; i < model.nodes.size(); i++) {
		auto &node = model.nodes[i];

		std::string nodename = node.name;

		model_info.node_names[i] = nodename;

		std::array<float, 16> matrix{};

		//node has a matrix
		if (!node.matrix.empty()) {
			for (int n = 0; n < 16; n++) {
				matrix[n] = (float)node.matrix[n];
			}

			//glm::mat4 flip = glm::mat4{ 1.0 };
			//flip[1][1] = -1;

			glm::mat4 mat;

			memcpy(&mat, &matrix, sizeof(glm::mat4));

			mat = mat; // * flip;

			memcpy(matrix.data(), &mat, sizeof(glm::mat4));
		}
		//separate transform
		else {
			glm::mat4 translation{ 1.f };
			if (!node.translation.empty()) {
				translation = glm::translate(
						glm::mat4(1.0f), glm::vec3{ node.translation[0], node.translation[1], node.translation[2] });
			}

			glm::mat4 rotation{ 1.f };

			if (!node.rotation.empty()) {
				glm::quat rot((float)node.rotation[3], (float)node.rotation[0], (float)node.rotation[1],
						(float)node.rotation[2]);
				rotation = glm::mat4{ rot };
			}

			glm::mat4 scale{ 1.f };
			if (!node.scale.empty()) {
				scale = glm::scale(glm::mat4(1.0f), glm::vec3{ node.scale[0], node.scale[1], node.scale[2] });
			}
			//glm::mat4 flip = glm::mat4{ 1.0 };
			//flip[1][1] = -1;

			glm::mat4 transform_matrix = (translation * rotation * scale); // * flip;

			memcpy(matrix.data(), &transform_matrix, sizeof(glm::mat4));
		}

		model_info.node_matrices[i] = (int)model_info.matrices.size();
		model_info.matrices.push_back(matrix);

		if (node.mesh >= 0) {
			auto mesh = model.meshes[node.mesh];

			if (mesh.primitives.size() > 1) {
				mesh_nodes.push_back(i);
			} else {
				auto primitive = mesh.primitives[0];
				std::string meshname = calculate_gltf_mesh_name(model, node.mesh, 0);

				fs::path meshpath = output_folder / (meshname + ".mesh");

				int material = primitive.material;

				std::string matname = calculate_gltf_material_name(model, material);

				fs::path materialpath = output_folder / (matname + ".mat");

				assets::ModelInfo::NodeMesh nmesh;
				nmesh.mesh_path = conv_state.convert_to_export_relative(meshpath).string();
				nmesh.material_path = conv_state.convert_to_export_relative(materialpath).string();

				model_info.node_meshes[i] = nmesh;
			}
		}
	}

	//calculate parent hierarchies
	//gltf stores children, but we want parent
	for (int i = 0; i < model.nodes.size(); i++) {
		for (auto c : model.nodes[i].children) {
			model_info.node_parents[c] = i;
		}
	}

	//for every gltf node that is a root node (no parents), apply the coordinate fixup

	glm::mat4 flip = glm::mat4{ 1.0 };
	flip[1][1] = -1;

	glm::mat4 rotation = glm::mat4{ 1.0 };
	//flip[1][1] = -1;
	rotation = glm::rotate(glm::mat4(1.0), glm::radians(-180.f), glm::vec3{ 1, 0, 0 });

	//flip[2][2] = -1;
	for (int i = 0; i < model.nodes.size(); i++) {
		auto it = model_info.node_parents.find(i);
		if (it == model_info.node_parents.end()) {
			auto matrix = model_info.matrices[model_info.node_matrices[i]];
			//no parent, root node
			glm::mat4 mat;

			memcpy(&mat, &matrix, sizeof(glm::mat4));

			mat = rotation * (flip * mat);

			memcpy(&matrix, &mat, sizeof(glm::mat4));

			model_info.matrices[model_info.node_matrices[i]] = matrix;
		}
	}

	int node_index = (int)model.nodes.size();
	//iterate nodes with mesh, convert each submesh into a node
	for (int i = 0; i < mesh_nodes.size(); i++) {
		auto &node = model.nodes[i];

		if (node.mesh < 0) {
			break;
		}

		auto mesh = model.meshes[node.mesh];

		for (int prim_index = 0; prim_index < mesh.primitives.size(); prim_index++) {
			auto primitive = mesh.primitives[prim_index];
			int newnode = node_index++;

			auto prim_index_string = std::to_string(prim_index);

			model_info.node_names[newnode] = model_info.node_names[i] + "_PRIM_" + prim_index_string;

			int material = primitive.material;
			auto mat = model.materials[material];
			std::string matname = calculate_gltf_material_name(model, material);
			std::string meshname = calculate_gltf_mesh_name(model, node.mesh, prim_index);

			fs::path materialpath = output_folder / (matname + ".mat");
			fs::path meshpath = output_folder / (meshname + ".mesh");

			assets::ModelInfo::NodeMesh nmesh;
			nmesh.mesh_path = conv_state.convert_to_export_relative(meshpath).string();
			nmesh.material_path = conv_state.convert_to_export_relative(materialpath).string();

			model_info.node_meshes[newnode] = nmesh;
		}
	}

	assets::AssetFile new_file = assets::pack_model(model_info);

	fs::path scenefilepath = (output_folder.parent_path()) / input.stem();

	scenefilepath.replace_extension(".mdl");

	//save to disk
	save_binary_file(scenefilepath.string().c_str(), new_file);
}

void extract_gltf_skinned_nodes(tinygltf::Model &model, const fs::path &input, const fs::path &output_folder,
		const ConverterState &conv_state) {
	assets::SkinnedModelInfo model_info;
	assets::JointData joint_data;
	for (tg::Skin &skin : model.skins) {
		tinygltf::Accessor &matrices_accessor = model.accessors[skin.inverseBindMatrices];

		std::vector<uint8_t> matrices_data;
		unpack_gltf_buffer(model, matrices_accessor, matrices_data);
		joint_data.rotation.resize(skin.joints.size());
		joint_data.translation.resize(skin.joints.size());
		joint_data.id.resize(skin.joints.size());
		model_info.joint_names.resize(skin.joints.size());
		for (int32_t i = 0; i < matrices_accessor.count; ++i) {
			glm::mat4 transform_matrix{};
			if (matrices_accessor.type == TINYGLTF_TYPE_MAT4 &&
					matrices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				memcpy(&transform_matrix, matrices_data.data() + i * sizeof(glm::mat4), sizeof(glm::mat4));
			} else {
				SPDLOG_ERROR("INVALID MATRIX TYPE");
				assert(false);
			}

			glm::vec3 place_holder;
			glm::vec4 place_holder2;

			glm::vec3 glm_translation;
			glm::quat glm_rotation;

			glm::decompose(transform_matrix, place_holder, glm_rotation, glm_translation, place_holder, place_holder2);
			std::array<float, 3> translation = { glm_translation[0], glm_translation[1], glm_translation[2] };
			std::array<float, 4> rotation = { glm_rotation[0], glm_rotation[1], glm_rotation[2], glm_rotation[3] };

			joint_data.translation[i] = translation;
			joint_data.rotation[i] = rotation;
			joint_data.id[i] = i;
			model_info.joint_names[i] = model.nodes[skin.joints[i]].name;
		}
		break; // Read only first skin, because idk how to use more than one skin, but maybe in the future
	}
	model_info.joint_rotation_buffer_size = joint_data.rotation.size() * sizeof(float) * 4;
	model_info.joint_translation_buffer_size = joint_data.translation.size() * sizeof(float) * 3;
	model_info.joint_id_buffer_size = joint_data.id.size() * sizeof(int32_t);

	assets::BoneData bone_data;

	bone_data.translation.resize(model.nodes.size());
	bone_data.rotation.resize(model.nodes.size());
	model_info.bone_names.resize(model.nodes.size());
	model_info.bone_parents.resize(model.nodes.size());
	// -1 means that node is root
	for (int32_t i = 0; i < model.nodes.size(); ++i) {
		model_info.bone_parents[i] = -1;
	}

	// Read only bone nodes
	for (int i = 0; i < model.nodes.size(); i++) {
		auto &node = model.nodes[i];

		std::string node_name = node.name;
		glm::mat4 transform_matrix;

		bool is_node_bone = !node.translation.empty() && !node.rotation.empty();
		if (is_node_bone) {
			glm::mat4 translation = glm::translate(
					glm::mat4(1.0f), glm::vec3{ node.translation[0], node.translation[1], node.translation[2] });

			glm::mat4 rotation = glm::mat4(glm::quat((float)node.rotation[3], (float)node.rotation[0],
					(float)node.rotation[1], (float)node.rotation[2]));

			transform_matrix = translation * rotation;
		} else if (!node.matrix.empty()) {
			for (int32_t j = 0; j < node.matrix.size(); j++) {
				transform_matrix[j / 4][j % 4] = node.matrix[j];
			}
		} else {
			transform_matrix = glm::mat4(1.0f);
		}

		glm::vec3 place_holder;
		glm::vec4 place_holder2;

		glm::vec3 glm_translation;
		glm::quat glm_rotation;

		glm::decompose(transform_matrix, place_holder, glm_rotation, glm_translation, place_holder, place_holder2);
		std::array<float, 3> translation = { glm_translation[0], glm_translation[1], glm_translation[2] };
		std::array<float, 4> rotation = { glm_rotation[0], glm_rotation[1], glm_rotation[2], glm_rotation[3] };

		bone_data.translation[i] = translation;
		bone_data.rotation[i] = rotation;
		model_info.bone_names[i] = node_name;
	}
	model_info.bone_rotation_buffer_size = bone_data.rotation.size() * sizeof(float) * 4;
	model_info.bone_translation_buffer_size = bone_data.translation.size() * sizeof(float) * 3;

	//calculate parent hierarchies
	//gltf stores children, but we want parent
	for (int i = 0; i < model.nodes.size(); i++) {
		for (int32_t child : model.nodes[i].children) {
			model_info.bone_parents[child] = i;
		}
	}

	//for every gltf node that is a root node (no parents), apply the coordinate fixup

	//	glm::mat4 flip = glm::mat4{ 1.0 };
	//	flip[1][1] = -1;
	//	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), glm::radians(-180.f), glm::vec3{ 1, 0, 0 });

	for (int32_t i = 0; i < model_info.bone_parents.size(); ++i) {
		if (model_info.bone_parents[i] == -1) {
			model_info.root = i;
			break;
			//			//no parent, root node
			//			glm::vec3 translation = { bone_data.translation[i][0], bone_data.translation[i][1],
			//				bone_data.translation[i][2] };
			//			glm::quat rotate = { bone_data.rotation[i][0], bone_data.rotation[i][1],
			//bone_data.rotation[i][2], 				bone_data.rotation[i][3] };
			//
			//			glm::mat4 matrix = glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotate);
			//			matrix = rotation * (flip * matrix);
			//
			//			glm::vec3 place_holder;
			//			glm::vec4 place_holder2;
			//
			//			glm::vec3 glm_translation;
			//			glm::quat glm_rotation;
			//
			//			glm::decompose(matrix, place_holder, glm_rotation, glm_translation, place_holder,
			//place_holder2); 			std::array<float, 3> t = { glm_translation[0], glm_translation[1],
			//glm_translation[2] }; 			std::array<float, 4> r = { glm_rotation[0], glm_rotation[1],
			//glm_rotation[2],
			//glm_rotation[3] }; 			bone_data.translation[i] = t; 			bone_data.rotation[i] = r;
		}
	}

	std::vector<uint64_t> mesh_nodes;
	for (int i = 0; i < model.nodes.size(); i++) {
		auto &node = model.nodes[i];

		if (node.mesh >= 0) {
			auto mesh = model.meshes[node.mesh];

			if (mesh.primitives.size() > 1) {
				mesh_nodes.push_back(i);
			} else {
				auto primitive = mesh.primitives[0];
				std::string meshname = calculate_gltf_mesh_name(model, node.mesh, 0);

				fs::path meshpath = output_folder / (meshname + ".skinned_mesh");

				int material = primitive.material;

				std::string matname = calculate_gltf_material_name(model, material);

				fs::path materialpath = output_folder / (matname + ".mat");

				assets::SkinnedModelInfo::NodeMesh nmesh;
				nmesh.mesh_path = conv_state.convert_to_export_relative(meshpath).string();
				nmesh.material_path = conv_state.convert_to_export_relative(materialpath).string();

				model_info.node_meshes[i] = nmesh;
			}
		}
	}

	int node_index = (int)model.nodes.size();
	//iterate nodes with mesh, convert each submesh into a node
	for (int i = 0; i < mesh_nodes.size(); i++) {
		auto &node = model.nodes[i];

		if (node.mesh < 0) {
			break;
		}

		auto mesh = model.meshes[node.mesh];

		for (int prim_index = 0; prim_index < mesh.primitives.size(); prim_index++) {
			auto primitive = mesh.primitives[prim_index];
			int newnode = node_index++;

			auto prim_index_string = std::to_string(prim_index);

			model_info.bone_names[newnode] = model_info.bone_names[i] + "_PRIM_" + prim_index_string;

			int material = primitive.material;
			auto mat = model.materials[material];
			std::string matname = calculate_gltf_material_name(model, material);
			std::string meshname = calculate_gltf_mesh_name(model, node.mesh, prim_index);

			fs::path materialpath = output_folder / (matname + ".mat");
			fs::path meshpath = output_folder / (meshname + ".skinned_mesh");

			assets::SkinnedModelInfo::NodeMesh nmesh;
			nmesh.mesh_path = conv_state.convert_to_export_relative(meshpath).string();
			nmesh.material_path = conv_state.convert_to_export_relative(materialpath).string();

			model_info.node_meshes[newnode] = nmesh;
		}
	}

	assets::AssetFile new_file = assets::pack_model(model_info, bone_data, joint_data);

	fs::path scene_filepath = (output_folder.parent_path()) / input.stem();

	scene_filepath.replace_extension(".skinmdl");

	//save to disk
	save_binary_file(scene_filepath.string().c_str(), new_file);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		SPDLOG_ERROR("You need to put a resources directory as an argument");
		return 1;
	}

	fs::path path{ argv[1] };

	const fs::path &asset_dir = path / "assets";
	const fs::path &skinned_meshes_dir = path / "skinned_assets";
	const fs::path &animations_dir = path / "animations";
	const fs::path &export_dir = path / "assets_export";
	const fs::path &tmp_dir = path / "assets_tmp";
	const fs::path &cubemap_dir = path / "cubemaps";

	if (!fs::is_directory(export_dir)) {
		fs::create_directory(export_dir);
	}

	if (!fs::is_directory(tmp_dir)) {
		fs::create_directory(tmp_dir);
	}

	if (!fs::is_directory(cubemap_dir)) {
		fs::create_directory(cubemap_dir);
	}

	ConverterState conv_state;
	conv_state.asset_path = path;
	conv_state.export_path = export_dir;

	SPDLOG_INFO("Loading asset asset_dir at {}", asset_dir.string());
	SPDLOG_INFO("Saving baked assets to {}", export_dir.string());

	std::vector<std::future<void>> futures = {};

	for (auto &p : fs::recursive_directory_iterator(asset_dir)) {
		SPDLOG_INFO("File: {}", p.path().string());

		auto relative = p.path().lexically_proximate(asset_dir);
		auto export_path = export_dir / relative;

		if (!fs::is_directory(export_path.parent_path())) {
			fs::create_directory(export_path.parent_path());
		}

		if (p.path().extension() == ".png" || p.path().extension() == ".jpg" || p.path().extension() == ".jpeg") {
			SPDLOG_INFO("found a texture");

			auto new_path = p.path();
			new_path.replace_extension(".ktx2");

			auto folder = export_path.parent_path() / new_path.filename().string();

			futures.push_back(std::async([p, folder]() { convert_image(p.path(), folder); }));
		}
		if (p.path().extension() == ".gltf") {
			SPDLOG_INFO("found a glTF model");

			using namespace tg;
			Model model;
			std::string err;
			std::string warn;

			TinyGLTF loader;

			bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, p.path().generic_string());

			if (!warn.empty()) {
				SPDLOG_WARN("Warn: %s\n", warn.c_str());
			}

			if (!err.empty()) {
				SPDLOG_ERROR("Err: %s\n", err.c_str());
				continue;
			}

			if (!ret) {
				SPDLOG_ERROR("Failed to load gltf file: {} - {} - {}", p.path().string(), err, warn);
				continue;
			}

			auto folder = export_path.parent_path() / (p.path().stem().string() + "_GLTF");
			fs::create_directory(folder);

			extract_gltf_meshes(model, p.path(), folder, conv_state);
			extract_gltf_materials(model, p.path(), folder, conv_state);
			extract_gltf_nodes(model, p.path(), folder, conv_state);
		}
	}

	for (auto &p : fs::recursive_directory_iterator(skinned_meshes_dir)) {
		SPDLOG_INFO("File: {}", p.path().string());

		auto relative = p.path().lexically_proximate(skinned_meshes_dir);
		auto export_path = export_dir / relative;

		if (!fs::is_directory(export_path.parent_path())) {
			fs::create_directory(export_path.parent_path());
		}

		if (p.path().extension() == ".png" || p.path().extension() == ".jpg" || p.path().extension() == ".jpeg") {
			SPDLOG_INFO("found a texture");

			auto new_path = p.path();
			new_path.replace_extension(".ktx2");

			auto folder = export_path.parent_path() / new_path.filename().string();

			futures.push_back(std::async([p, folder]() { convert_image(p.path(), folder); }));
		}

		if (p.path().extension() == ".gltf") {
			SPDLOG_INFO("found a glTF model");

			using namespace tg;
			Model model;
			std::string err;
			std::string warn;

			TinyGLTF loader;

			bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, p.path().generic_string());

			if (!warn.empty()) {
				SPDLOG_WARN("Warn: %s\n", warn.c_str());
			}

			if (!err.empty()) {
				SPDLOG_ERROR("Err: %s\n", err.c_str());
				continue;
			}

			if (!ret) {
				SPDLOG_ERROR("Failed to load gltf file: {} - {} - {}", p.path().string(), err, warn);
				continue;
			}

			auto folder = export_path.parent_path() / (p.path().stem().string() + "_GLTF");
			fs::create_directory(folder);

			extract_gltf_skinned_meshes(model, p.path(), folder, conv_state);
			extract_gltf_materials(model, p.path(), folder, conv_state);
			extract_gltf_skinned_nodes(model, p.path(), folder, conv_state);
		}
	}

	for (auto &p : fs::recursive_directory_iterator(animations_dir)) {
		SPDLOG_INFO("File: {}", p.path().string());

		auto relative = p.path().lexically_proximate(animations_dir);
		auto export_path = export_dir / relative;

		if (!fs::is_directory(export_path.parent_path())) {
			fs::create_directory(export_path.parent_path());
		}

		if (p.path().extension() == ".gltf") {
			SPDLOG_INFO("found a glTF model");

			using namespace tg;
			Model model;
			std::string err;
			std::string warn;

			TinyGLTF loader;

			bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, p.path().generic_string());

			if (!warn.empty()) {
				SPDLOG_WARN("Warn: %s\n", warn.c_str());
			}

			if (!err.empty()) {
				SPDLOG_ERROR("Err: %s\n", err.c_str());
				continue;
			}

			if (!ret) {
				SPDLOG_ERROR("Failed to load gltf file: {} - {} - {}", p.path().string(), err, warn);
				continue;
			}

			if (model.animations.empty()) {
				SPDLOG_ERROR("Model does not have any animations: {}", p.path().string());
				continue;
			}

			// Read animations
			for (Animation &animation : model.animations) {
				auto folder = export_path.parent_path() / (p.path().stem().string() + "_ANIM_GLTF");

				fs::create_directory(folder);

				extract_gltf_animations(model, p.path().stem().string(), animation, p.path(), folder, conv_state);
			}
		}
	}

	for (auto &p : fs::directory_iterator(cubemap_dir)) {
		SPDLOG_INFO("Loading cubemap at {}", p.path().string());

		auto px = p.path() / "px.png";
		auto nx = p.path() / "nx.png";
		auto py = p.path() / "py.png";
		auto ny = p.path() / "ny.png";
		auto pz = p.path() / "pz.png";
		auto nz = p.path() / "nz.png";

		auto relative = p.path().lexically_proximate(cubemap_dir);
		auto filename = p.path().filename().string();
		auto export_path = export_dir / "cubemaps" / filename;

		if (!fs::is_directory(export_path.parent_path())) {
			fs::create_directory(export_path.parent_path());
		}

		if (!fs::is_directory(export_path)) {
			fs::create_directory(export_path);
		}

		fs::path output = export_path / "environment_map.ktx2";

		auto input = fmt::format(
				"{} {} {} {} {} {}", px.string(), nx.string(), py.string(), ny.string(), pz.string(), nz.string());

		auto result =
				system(fmt::format("toktx --encode uastc --genmipmap --cubemap {} {}", output.string(), input).c_str());
	}

	//convert skyboxes
	OpenGLContext context = {};
	context.startup();

	for (auto &p : fs::directory_iterator(export_dir / "cubemaps")) {
		SPDLOG_INFO("Loading cubemap at {}", p.path().string());

		auto file = p.path() / "environment_map.ktx2";

		context.process_cubemap(file.generic_string(), tmp_dir.generic_string());

		auto px = tmp_dir / "px.png";
		auto nx = tmp_dir / "nx.png";
		auto py = tmp_dir / "py.png";
		auto ny = tmp_dir / "ny.png";
		auto pz = tmp_dir / "pz.png";
		auto nz = tmp_dir / "nz.png";

		// Create irradiance map texture
		fs::path output = p.path() / "irradiance_map.ktx2";

		std::string input = fmt::format(
				"{} {} {} {} {} {}", px.string(), nx.string(), py.string(), ny.string(), pz.string(), nz.string());

		auto result =
				system(fmt::format("toktx --encode uastc --genmipmap --cubemap {} {}", output.string(), input).c_str());

		// Create prefilter map texture
		output = p.path() / "prefilter_map.ktx2";

		input.clear();
		for (int i = 0; i < 5; i++) {
			input += (tmp_dir / ("mip_" + std::to_string(i) + "_px.png ")).string();
		}
		for (int i = 0; i < 5; i++) {
			input += (tmp_dir / ("mip_" + std::to_string(i) + "_nx.png ")).string();
		}
		for (int i = 0; i < 5; i++) {
			input += (tmp_dir / ("mip_" + std::to_string(i) + "_py.png ")).string();
		}
		for (int i = 0; i < 5; i++) {
			input += (tmp_dir / ("mip_" + std::to_string(i) + "_ny.png ")).string();
		}
		for (int i = 0; i < 5; i++) {
			input += (tmp_dir / ("mip_" + std::to_string(i) + "_pz.png ")).string();
		}
		for (int i = 0; i < 5; i++) {
			input += (tmp_dir / ("mip_" + std::to_string(i) + "_nz.png ")).string();
		}

		result = system(fmt::format("toktx --encode uastc --levels 5 --convert_oetf linear --mipmap --cubemap {} {}",
				output.string(), input)
								.c_str());

		// Create brdf lut texture
		output = p.path() / "brdf_lut.ktx2";

		auto brdf_input = tmp_dir / "brdf_lut.png";

		result = system(
				fmt::format("toktx --encode uastc --genmipmap {} {}", output.string(), brdf_input.string()).c_str());
	}

	for (auto &f : futures) {
		f.wait();
	}
}