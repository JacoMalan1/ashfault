#ifndef ASHFAULT_ASSET_MANAGER_H
#define ASHFAULT_ASSET_MANAGER_H

#include <ashfault/ashfault.h>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <string>
#include <stdexcept>

namespace ashfault {
class ASHFAULT_API IAsset {
public:
  virtual void destroy() = 0;
};

template <class T>
  requires std::is_base_of<IAsset, T>::value
class ASHFAULT_API Asset {
public:
  Asset() = default;
  Asset(const std::string &id, const std::string &path,
        std::shared_ptr<T> asset)
      : m_ID(id), m_Path(path), m_Asset(asset) {}
  ~Asset() = default;

  std::shared_ptr<T> get() { return m_Asset; }
  std::shared_ptr<T> get() const { return m_Asset; }
  const std::string &id() const { return m_ID; }
  const std::string &path() const { return m_Path; }
  void destroy() { m_Asset->destroy(); }

private:
  std::string m_ID;
  std::string m_Path;
  std::shared_ptr<T> m_Asset;
};

class ASHFAULT_API IAssetLoader {
public:
  IAssetLoader() = default;
  virtual void destroy() = 0;
};

template <class T>
class ASHFAULT_API AssetLoader : public IAssetLoader {
public:
  AssetLoader() = default;
  virtual ~AssetLoader() = default;

  Asset<T> load(const std::string &id, const std::string &path) {
    if (m_LoadedAssets.count(id) > 0) {
      return m_LoadedAssets.at(id);
    }

    Asset<T> asset(id, path, this->read(path));
    m_LoadedAssets.insert(std::make_pair(id, asset));
    return asset;
  }

  void reload(const std::string &id) {
    if (m_LoadedAssets.count(id) > 0) {
      auto new_asset = read(m_LoadedAssets[id].path());
      *m_LoadedAssets[id].get() = *new_asset;
    }
  }

  void destroy() override {
    for (auto [name, asset] : m_LoadedAssets) {
      asset.destroy();
    }
  }

protected:
  virtual std::shared_ptr<T> read(const std::string &path) = 0;

private:
  std::unordered_map<std::string, Asset<T>> m_LoadedAssets;
};

class ASHFAULT_API AssetManager {
public:
  template <class T>
  void register_loader(std::shared_ptr<AssetLoader<T>> loader) {
    std::type_index type_idx = typeid(T);
    if (m_Loaders.count(type_idx) > 0) {
      throw std::runtime_error("Attempt to register duplicate asset loader");
    }

    m_Loaders.insert(
        std::make_pair(type_idx, std::static_pointer_cast<void>(loader)));
  }

  template <class T>
  Asset<T> load(const std::string &id, const std::string &path) {
    std::type_index type_idx = typeid(T);
    if (m_Loaders.count(type_idx) == 0) {
      throw std::runtime_error("No loader registered for asset type");
    }

    std::shared_ptr<AssetLoader<T>> loader =
        std::static_pointer_cast<AssetLoader<T>>(m_Loaders[type_idx]);
    return loader->load(id, path);
  }

  template <class T>
  void reload(const std::string &id) {
    std::type_index type_idx = typeid(T);
    if (m_Loaders.count(type_idx) == 0) {
      throw std::runtime_error("No loader registered for asset type");
    }

    std::shared_ptr<AssetLoader<T>> loader =
        std::static_pointer_cast<AssetLoader<T>>(m_Loaders[type_idx]);
    return loader->reload(id);
  }

  void destroy() {
    for (auto [idx, loader] : m_Loaders) {
      std::static_pointer_cast<IAssetLoader>(loader)->destroy();
    }
  }

private:
  std::unordered_map<std::type_index, std::shared_ptr<void>> m_Loaders;
};
}  // namespace ashfault

#endif
