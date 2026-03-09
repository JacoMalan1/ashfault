#ifndef ASHFAULT_APPLICATION_H
#define ASHFAULT_APPLICATION_H

#include <CLSTL/shared_ptr.h>
#include <ashfault/core/engine.h>
#include <ashfault/core/input.h>

namespace ashfault {
class Application {
public:
  Application(clstl::shared_ptr<Engine> engine,
              clstl::shared_ptr<Window> window);
  virtual ~Application() = default;

  virtual void run();

protected:
  clstl::shared_ptr<Engine> m_Engine;
  clstl::shared_ptr<Window> m_Window;
  clstl::shared_ptr<Input> m_Input;
};
} // namespace ashfault

#endif
