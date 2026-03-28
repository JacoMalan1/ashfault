#include <ashfault/core/asset/script_loader.h>
#include <fstream>
#include <memory>

namespace ashfault {
ScriptLoader::ScriptLoader() : AssetLoader<Script>() {}

std::shared_ptr<Script> ScriptLoader::read(const std::string &path) {
  std::ifstream fs(path);

  if (!fs.is_open()) {
    throw std::runtime_error("Failed to read script!");
  }

  fs.seekg(0, std::ios_base::end);
  auto size = fs.tellg();
  fs.seekg(0, std::ios_base::beg);

  std::string contents;
  contents.resize(size);
  fs.read(contents.data(), size);
  fs.close();
  return std::make_shared<Script>(contents);
}
}  // namespace ashfault
