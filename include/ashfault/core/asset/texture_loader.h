#ifndef ASHFAULT_ASSET_TEXTURE_LOADER_H
#define ASHFAULT_ASSET_TEXTURE_LOADER_H

#include <ashfault/core/asset_manager.hpp>
#include <ashfault/core/script.h>
#include <ashfault/core/texture.h>
#include <ashfault/renderer/vkrenderer.h>

namespace ashfault {
class TextureLoader : public AssetLoader<Texture> {
public:
  TextureLoader() = default;
  ~TextureLoader() = default;

private:
  std::shared_ptr<Texture> read(const std::string &path) override;
};
}  // namespace ashfault

#endif
