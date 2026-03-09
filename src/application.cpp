#include <CLSTL/shared_ptr.h>
#include <ashfault/application.h>

namespace ashfault {
Application::Application(clstl::shared_ptr<Engine> engine,
                         clstl::shared_ptr<Window> window)
    : m_Engine(engine), m_Window(window),
      m_Input(clstl::make_shared<Input>(window)) {
  engine->setup_renderer(window);
}

void Application::run() {}
} // namespace ashfault
