#include <ashfault/core/asset/texture_loader.h>
#include <ashfault/renderer/renderer.h>
#include <stb_image.h>

namespace ashfault {
std::shared_ptr<Texture> TextureLoader::read(const std::string &path) {
  int width, height, channels;
  auto *image = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
  if (!image) {
    throw std::runtime_error("Failed to read texture");
  }
  auto index = Renderer::upload_texture(reinterpret_cast<const char *>(image), width,
                           height);
  stbi_image_free(image);
  return std::make_shared<Texture>(index);
}
}  // namespace ashfault
