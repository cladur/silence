#ifndef SILENCE_MATERIAL_ASSET_H
#define SILENCE_MATERIAL_ASSET_H

#include "asset_loader.h"

namespace assets {

enum class TransparencyMode : uint8_t { Opaque, Transparent, Masked };

struct MaterialInfo {
	std::string base_effect;
	std::unordered_map<std::string, std::string> textures; //name -> path
	std::unordered_map<std::string, std::string> custom_properties;
	TransparencyMode transparency;
};

MaterialInfo read_material_info(AssetFile *file);

AssetFile pack_material(MaterialInfo *info);
} //namespace assets

#endif //SILENCE_MATERIAL_ASSET_H
