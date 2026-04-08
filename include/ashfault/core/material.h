#ifndef ASHFAULT_MATERIAL_H
#define ASHFAULT_MATERIAL_H

#include <ashfault/core/asset_manager.hpp>
#include <optional>
#include <ashfault/core/texture.h>

namespace ashfault {
struct Material {
  float diffuse;
  float specular;
  std::optional<Asset<Texture>> albedo_texture;
  std::optional<Asset<Texture>> normal_texture;
};
}  // namespace ashfault

#endif
