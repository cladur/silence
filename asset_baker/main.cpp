#include "asset_loader.h"
#include "mesh_asset.h"
#include "texture_asset.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

namespace tg = tinygltf;
namespace fs = std::filesystem;
using namespace assets;

bool convert_image(const fs::path &input, const fs::path &output) {
	SPDLOG_INFO("Converting image {} to {}", input.string(), output.string());

	int tex_width, tex_height, tex_channels;

	stbi_uc *pixels =
			stbi_load((const char *)input.u8string().c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

	if (!pixels) {
		SPDLOG_ERROR("Failed to load texture file {}", input.string());
		return false;
	}

	int texture_size = tex_width * tex_height * 4;

	TextureInfo tex_info;
	tex_info.texture_size = texture_size;
	tex_info.pixel_size[0] = tex_width;
	tex_info.pixel_size[1] = tex_height;
	tex_info.texture_format = TextureFormat::RGBA8;
	tex_info.original_file = input.string();
	assets::AssetFile new_image = assets::pack_texture(&tex_info, pixels);

	stbi_image_free(pixels);

	save_binary_file(output.string().c_str(), new_image);

	return true;
}

void pack_vertex(assets::VertexPNCV32 &new_vert, float vx, float vy, float vz, float nx, float ny, float nz, float ux,
		float uy) {
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

template <typename V>
void extract_mesh_from_gltf(
		const tg::Model &model, const tg::Mesh &mesh, std::vector<V> &vertices, std::vector<uint32_t> &indices) {
	for (const auto &primitive : mesh.primitives) {
		const auto &attributes = primitive.attributes;

		// Load vertices
		const auto &position_accessor = model.accessors[attributes.find("POSITION")->second];
		const auto &position_buffer_view = model.bufferViews[position_accessor.bufferView];
		const auto &position_buffer = model.buffers[position_buffer_view.buffer];

		const auto &normal_accessor = model.accessors[attributes.find("NORMAL")->second];
		const auto &normal_buffer_view = model.bufferViews[normal_accessor.bufferView];
		const auto &normal_buffer = model.buffers[normal_buffer_view.buffer];

		const auto &uv_accessor = model.accessors[attributes.find("TEXCOORD_0")->second];
		const auto &uv_buffer_view = model.bufferViews[uv_accessor.bufferView];
		const auto &uv_buffer = model.buffers[uv_buffer_view.buffer];

		const auto &position_data = reinterpret_cast<const float *>(
				&position_buffer.data[position_buffer_view.byteOffset + position_accessor.byteOffset]);
		const auto &normal_data = reinterpret_cast<const float *>(
				&normal_buffer.data[normal_buffer_view.byteOffset + normal_accessor.byteOffset]);
		const auto &uv_data =
				reinterpret_cast<const float *>(&uv_buffer.data[uv_buffer_view.byteOffset + uv_accessor.byteOffset]);

		for (size_t i = 0; i < position_accessor.count; i++) {
			auto pos = glm::vec3(position_data[i * 3], position_data[i * 3 + 1], position_data[i * 3 + 2]);
			auto normal = glm::vec3(normal_data[i * 3], normal_data[i * 3 + 1], normal_data[i * 3 + 2]);
			auto color = normal;
			auto uv = glm::vec2(uv_data[i * 2], uv_data[i * 2 + 1]);

			V vertex = {};
			pack_vertex(vertex, pos.x, pos.y, pos.z, normal.x, normal.y, normal.z, uv.x, uv.y);
			vertices.push_back(vertex);
		}

		// Load indices
		const auto &index_accessor = model.accessors[primitive.indices];
		const auto &index_buffer_view = model.bufferViews[index_accessor.bufferView];
		const auto &gltf_index_buffer = model.buffers[index_buffer_view.buffer];

		const auto &index_data = &gltf_index_buffer.data[index_buffer_view.byteOffset + index_accessor.byteOffset];

		for (size_t i = 0; i < index_accessor.count; i++) {
			switch (index_accessor.componentType) {
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
					indices.push_back(reinterpret_cast<const uint32_t *>(index_data)[i]);
					break;
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
					indices.push_back(reinterpret_cast<const uint16_t *>(index_data)[i]);
					break;
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
					indices.push_back(reinterpret_cast<const uint8_t *>(index_data)[i]);
					break;
				default:
					break;
			}
		}
	}
}

bool convert_mesh(const fs::path &input, const fs::path &output) {
	SPDLOG_INFO("Converting mesh {} to {}", input.string(), output.string());

	tg::Model model;
	std::string err;
	std::string warn;

	static tg::TinyGLTF loader;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, input);

	if (!ret) {
		SPDLOG_ERROR("Failed to load gltf file: {} - {} - {}", input.string(), err, warn);
		return false;
	}

	if (!warn.empty()) {
		SPDLOG_WARN("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		SPDLOG_ERROR("Err: %s\n", err.c_str());
		return false;
	}

	using VertexFormat = assets::VertexPNCV32;
	auto vertex_format_enum = assets::VertexFormat::PNCV32;

	std::vector<VertexFormat> vertices;
	std::vector<uint32_t> indices;

	// TODO: Store meshes seperately instead of combining them
	for (const auto &mesh : model.meshes) {
		extract_mesh_from_gltf(model, mesh, vertices, indices);
	}

	MeshInfo mesh_info = {};
	mesh_info.vertex_buffer_size = vertices.size() * sizeof(VertexFormat);
	mesh_info.index_buffer_size = indices.size() * sizeof(uint32_t);
	// TODO: Calculate mesh bounds
	// mesh_info.bounds = ;
	mesh_info.vertex_format = vertex_format_enum;
	mesh_info.index_size = sizeof(uint32_t);
	mesh_info.compression_mode = assets::CompressionMode::LZ4;
	mesh_info.original_file = input.string();

	assets::AssetFile new_file = assets::pack_mesh(&mesh_info, (char *)vertices.data(), (char *)indices.data());

	save_binary_file(output.string().c_str(), new_file);

	return true;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		SPDLOG_ERROR("You need to put a asset directory as an argument");
		return 1;
	}

	fs::path path{ argv[1] };

	const fs::path &directory = path;

	SPDLOG_INFO("Loading asset directory at {}", directory.string());

	for (auto &p : fs::directory_iterator(directory)) {
		SPDLOG_INFO("File: {}", p.path().string());

		if (p.path().extension() == ".png") {
			SPDLOG_INFO("found a texture");

			auto new_path = p.path();
			new_path.replace_extension(".tx");
			convert_image(p.path(), new_path);
		}
		if (p.path().extension() == ".gltf") {
			SPDLOG_INFO("found a mesh");

			auto new_path = p.path();
			new_path.replace_extension(".mesh");
			convert_mesh(p.path(), new_path);
		}
	}
}