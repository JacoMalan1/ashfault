#ifndef ASHFAULT_ENGINE_H
#define ASHFAULT_ENGINE_H

#include <CLSTL/unique_ptr.h>
#include <ashfault/renderer.h>

namespace ashfault {
class Engine {
public:
  Engine();
  Engine(const Engine &) = delete;
  Engine &operator=(const Engine &) = delete;
  ~Engine();

  void run();

private:

  clstl::unique_ptr<Renderer> m_Renderer;
};
}

#endif
