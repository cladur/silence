#ifndef SILENCE_SKINNED_MESH_ASSET_H
#define SILENCE_SKINNED_MESH_ASSET_H

#include "asset_loader.h"
namespace assets {

struct SkinnedVertexPNV32 {
	float position[3];
	float normal[3];
	float uv[2];
	float weight[4];
	int32_t joint_id[4];
};

struct SkinnedVertexP32N8C8V16 {
	uint8_t normal[3];
	uint8_t color[3];
	uint8_t weight[4];
	float position[3];
	float uv[2];
	int32_t joint_id[4];
};

enum class SkinnedVertexFormat : uint32_t {
	Unknown = 0,
	PNV32, //everything at 32 bits
	P32N8C8V16 //position at 32 bits, normal at 8 bits, color at 8 bits, uvs at 16 bits float
};

struct SkinnedMeshBounds {
	float origin[3];
	float radius;
	float extents[3];
};

struct SkinnedMeshInfo {
	uint64_t vertex_buffer_size;
	uint64_t index_buffer_size;
	SkinnedMeshBounds bounds;
	SkinnedVertexFormat vertex_format;
	uint8_t index_size;
	CompressionMode compression_mode;
	std::string original_file;
};

SkinnedMeshInfo read_skinned_mesh_info(AssetFile *file);

void unpack_mesh(
		SkinnedMeshInfo *info, const char *source_buffer, size_t source_size, char *vertex_bufer, char *index_buffer);

AssetFile pack_mesh(SkinnedMeshInfo *info, char *vertex_data, char *index_data);

SkinnedMeshBounds calculate_bounds(SkinnedVertexPNV32 *vertices, size_t count);
} //namespace assets

#endif //SILENCE_SKINNED_MESH_ASSET_H
