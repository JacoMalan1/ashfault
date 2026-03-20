#include <ashfault/application.h>
#include <ashfault/renderer/renderer.h>

namespace ashfault {
Application::Application(std::shared_ptr<Window> window)
    : m_Window(window),
      m_Input(std::make_shared<Input>(window)),
      m_LayerStack(std::make_unique<LayerStack>()) {}

void Application::run() { Renderer::init(m_Window); }
}  // namespace ashfault
