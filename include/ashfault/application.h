#ifndef ASHFAULT_APPLICATION_H
#define ASHFAULT_APPLICATION_H

#include <ashfault/ashfault.h>
#include <ashfault/core/layer_stack.h>
#include <ashfault/core/window.h>

#include <memory>
#include <ashfault/core/asset_manager.hpp>

namespace ashfault {
class ASHFAULT_API Application {
public:
  Application(std::shared_ptr<Window> window);
  virtual ~Application() = default;

  virtual void run();

protected:
  std::shared_ptr<Window> m_Window;
  LayerStack *m_LayerStack;
  std::shared_ptr<AssetManager> m_AssetManager;
};
}  // namespace ashfault

#endif
