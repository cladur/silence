#include "mesh_asset.h"

#include "lz4.h"
#include "mesh_asset.h"
#include <nlohmann/json.hpp>

assets::VertexFormat parse_format(const char *f) {
	if (strcmp(f, "PNCV32") == 0) {
		return assets::VertexFormat::PNCV_F32;
	} else if (strcmp(f, "P32N8C8V16") == 0) {
		return assets::VertexFormat::P32N8C8V16;
	} else {
		return assets::VertexFormat::Unknown;
	}
}

assets::MeshInfo assets::read_mesh_info(AssetFile *file) {
	MeshInfo info;

	nlohmann::json metadata = nlohmann::json::parse(file->json);

	info.vertex_buffer_size = metadata["vertex_buffer_size"];
	info.index_buffer_size = metadata["index_buffer_size"];
	info.index_size = (uint8_t)metadata["index_size"];
	info.original_file = metadata["original_file"];

	std::string compression_string = metadata["compression"];
	info.compression_mode = parse_compression(compression_string.c_str());

	std::vector<float> bounds_data;
	bounds_data.reserve(7);
	bounds_data = metadata["bounds"].get<std::vector<float>>();

	info.bounds.origin[0] = bounds_data[0];
	info.bounds.origin[1] = bounds_data[1];
	info.bounds.origin[2] = bounds_data[2];

	info.bounds.radius = bounds_data[3];

	info.bounds.extents[0] = bounds_data[4];
	info.bounds.extents[1] = bounds_data[5];
	info.bounds.extents[2] = bounds_data[6];

	std::string vertex_format = metadata["vertex_format"];
	info.vertex_format = parse_format(vertex_format.c_str());
	return info;
}

void assets::unpack_mesh(
		MeshInfo *info, const char *sourcebuffer, size_t sourceSize, char *vertexBufer, char *indexBuffer) {
	//decompressing into temporal vector. TODO: streaming decompress directly on the buffers
	std::vector<char> decompressed_buffer;
	decompressed_buffer.resize(info->vertex_buffer_size + info->index_buffer_size);

	LZ4_decompress_safe(sourcebuffer, decompressed_buffer.data(), static_cast<int>(sourceSize),
			static_cast<int>(decompressed_buffer.size()));

	//copy vertex buffer
	memcpy(vertexBufer, decompressed_buffer.data(), info->vertex_buffer_size);

	//copy index buffer
	memcpy(indexBuffer, decompressed_buffer.data() + info->vertex_buffer_size, info->index_buffer_size);
}

assets::AssetFile assets::pack_mesh(MeshInfo *info, char *vertexData, char *indexData) {
	AssetFile file;
	file.type[0] = 'M';
	file.type[1] = 'E';
	file.type[2] = 'S';
	file.type[3] = 'H';
	file.version = 1;

	nlohmann::json metadata;
	if (info->vertex_format == VertexFormat::P32N8C8V16) {
		metadata["vertex_format"] = "P32N8C8V16";
	} else if (info->vertex_format == VertexFormat::PNCV_F32) {
		metadata["vertex_format"] = "PNCV_F32";
	}
	metadata["vertex_buffer_size"] = info->vertex_buffer_size;
	metadata["index_buffer_size"] = info->index_buffer_size;
	metadata["index_size"] = info->index_size;
	metadata["original_file"] = info->original_file;

	std::vector<float> bounds_data;
	bounds_data.resize(7);

	bounds_data[0] = info->bounds.origin[0];
	bounds_data[1] = info->bounds.origin[1];
	bounds_data[2] = info->bounds.origin[2];

	bounds_data[3] = info->bounds.radius;

	bounds_data[4] = info->bounds.extents[0];
	bounds_data[5] = info->bounds.extents[1];
	bounds_data[6] = info->bounds.extents[2];

	metadata["bounds"] = bounds_data;

	size_t fullsize = info->vertex_buffer_size + info->index_buffer_size;

	std::vector<char> merged_buffer;
	merged_buffer.resize(fullsize);

	//copy vertex buffer
	memcpy(merged_buffer.data(), vertexData, info->vertex_buffer_size);

	//copy index buffer
	memcpy(merged_buffer.data() + info->vertex_buffer_size, indexData, info->index_buffer_size);

	//compress buffer and copy it into the file struct
	size_t compress_staging = LZ4_compressBound(static_cast<int>(fullsize));

	file.binary_blob.resize(compress_staging);

	int compressed_size = LZ4_compress_default(merged_buffer.data(), file.binary_blob.data(),
			static_cast<int>(merged_buffer.size()), static_cast<int>(compress_staging));
	file.binary_blob.resize(compressed_size);

	metadata["compression"] = "LZ4";

	file.json = metadata.dump();

	return file;
}

assets::MeshBounds assets::calculate_bounds(VertexPNCV32 *vertices, size_t count) {
	MeshBounds bounds{};

	float min[3] = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max() };
	float max[3] = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(),
		std::numeric_limits<float>::min() };

	for (int i = 0; i < count; i++) {
		min[0] = std::min(min[0], vertices[i].position[0]);
		min[1] = std::min(min[1], vertices[i].position[1]);
		min[2] = std::min(min[2], vertices[i].position[2]);

		max[0] = std::max(max[0], vertices[i].position[0]);
		max[1] = std::max(max[1], vertices[i].position[1]);
		max[2] = std::max(max[2], vertices[i].position[2]);
	}

	bounds.extents[0] = (max[0] - min[0]) / 2.0f;
	bounds.extents[1] = (max[1] - min[1]) / 2.0f;
	bounds.extents[2] = (max[2] - min[2]) / 2.0f;

	bounds.origin[0] = bounds.extents[0] + min[0];
	bounds.origin[1] = bounds.extents[1] + min[1];
	bounds.origin[2] = bounds.extents[2] + min[2];

	//go through the vertices again to calculate the exact bounding sphere radius
	float r2 = 0;
	for (int i = 0; i < count; i++) {
		float offset[3];
		offset[0] = vertices[i].position[0] - bounds.origin[0];
		offset[1] = vertices[i].position[1] - bounds.origin[1];
		offset[2] = vertices[i].position[2] - bounds.origin[2];

		//pithagoras
		float distance = offset[0] * offset[0] + offset[1] * offset[1] + offset[2] * offset[2];
		r2 = std::max(r2, distance);
	}

	bounds.radius = std::sqrt(r2);

	return bounds;
}