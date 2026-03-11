#ifndef ASHFAULT_APPLICATION_H
#define ASHFAULT_APPLICATION_H

#include <ashfault/core/engine.h>
#include <ashfault/core/input.h>
#include <memory>

namespace ashfault {
class Application {
public:
  Application(std::shared_ptr<Engine> engine,
              std::shared_ptr<Window> window);
  virtual ~Application() = default;

  virtual void run();

protected:
  std::shared_ptr<Engine> m_Engine;
  std::shared_ptr<Window> m_Window;
  std::shared_ptr<Input> m_Input;
};
} // namespace ashfault

#endif
