#ifndef SILENCE_MESH_ASSET_H
#define SILENCE_MESH_ASSET_H

#include "asset_loader.h"

namespace assets {

struct VertexPNCV32 {
	float position[3];
	float normal[3];
	float color[3];
	float uv[2];
};
struct VertexP32N8C8V16 {
	float position[3];
	uint8_t normal[3];
	uint8_t color[3];
	float uv[2];
};

enum class VertexFormat : uint32_t {
	Unknown = 0,
	PNCV32, //everything at 32 bits
	P32N8C8V16 //position at 32 bits, normal at 8 bits, color at 8 bits, uvs at 16 bits float
};

struct MeshBounds {
	float origin[3];
	float radius;
	float extents[3];
};

struct MeshInfo {
	uint64_t vertex_buffer_size;
	uint64_t index_buffer_size;
	MeshBounds bounds;
	VertexFormat vertex_format;
	uint8_t index_size;
	CompressionMode compression_mode;
	std::string original_file;
};

MeshInfo read_mesh_info(AssetFile *file);

void unpack_mesh(MeshInfo *info, const char *source_buffer, size_t source_size, char *vertex_bufer, char *index_buffer);

AssetFile pack_mesh(MeshInfo *info, char *vertex_data, char *index_data);

MeshBounds calculate_bounds(VertexPNCV32 *vertices, size_t count);
} //namespace assets

#endif //SILENCE_MESH_ASSET_H
