#ifndef ASHFAULT_SCRIPT_LOADER_H
#define ASHFAULT_SCRIPT_LOADER_H

#include <ashfault/ashfault.h>
#include <ashfault/core/script.h>

#include <ashfault/core/asset_manager.hpp>
#include <memory>

namespace ashfault {
class ASHFAULT_API ScriptLoader : public AssetLoader<Script> {
public:
  ScriptLoader();
  ~ScriptLoader() = default;

protected:
  std::shared_ptr<Script> read(const std::string &path) override;
};
}  // namespace ashfault

#endif
