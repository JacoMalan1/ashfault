#include <ashfault/application.h>

namespace ashfault {
Application::Application(std::shared_ptr<Engine> engine,
                         std::shared_ptr<Window> window)
    : m_Engine(engine), m_Window(window),
      m_Input(std::make_shared<Input>(window)) {
  engine->setup_renderer(window);
}

void Application::run() {}
} // namespace ashfault
