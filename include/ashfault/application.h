#ifndef ASHFAULT_APPLICATION_H
#define ASHFAULT_APPLICATION_H

#include <ashfault/core/layer_stack.h>
#include <ashfault/core/input.h>
#include <ashfault/ashfault.h>
#include <memory>

namespace ashfault {
class ASHFAULT_API Application {
public:
  Application(std::shared_ptr<Window> window);
  virtual ~Application() = default;

  virtual void run();

protected:
  std::shared_ptr<Window> m_Window;
  std::shared_ptr<Input> m_Input;
  std::unique_ptr<LayerStack> m_LayerStack;
};
} // namespace ashfault

#endif
