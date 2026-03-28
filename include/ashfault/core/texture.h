#ifndef ASHFAULT_CORE_TEXTURE_H
#define ASHFAULT_CORE_TEXTURE_H

#include <ashfault/core/asset_manager.hpp>
#include <cstdint>

namespace ashfault {
class Texture : public IAsset {
public:
  Texture(std::uint32_t idx);
  void destroy() override;

  std::uint32_t index() const;

private:
  std::uint32_t m_Index;
};
}

#endif
