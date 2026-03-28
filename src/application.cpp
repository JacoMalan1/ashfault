#include <ashfault/application.h>
#include <ashfault/renderer/renderer.h>
#include <spdlog/spdlog.h>

namespace ashfault {
Application::Application(std::shared_ptr<Window> window)
    : m_Window(window),
      m_Input(std::make_shared<Input>(window)),
      m_LayerStack(new LayerStack()),
      m_AssetManager(std::make_shared<AssetManager>()) {}

void Application::run() {
  Renderer::init(m_Window);

  while (!m_Window->should_close()) {
    if (!Renderer::start_frame()) {
      SPDLOG_WARN("Couldn't start frame!");
      continue;
    }

    Renderer::end_frame();
    Renderer::submit_and_wait();
    m_Window->poll_events();
  }

  delete m_LayerStack;
  m_AssetManager->destroy();
  Renderer::shutdown();
}
}  // namespace ashfault
