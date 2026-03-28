#include <ashfault/core/texture.h>

namespace ashfault {
Texture::Texture(std::uint32_t index) : m_Index(index) {}

std::uint32_t Texture::index() const {
  return m_Index;
}

void Texture::destroy() { }
}
