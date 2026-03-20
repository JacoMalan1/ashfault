#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <ashfault/core/event/key_press.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <ashfault/core/layer/render_layer.h>
#include <ashfault/core/timer.hpp>
#include <ashfault/editor/editor.h>
#include <ashfault/editor/editor_layer.h>
#include <ashfault/editor/ui_layer.h>
#include <ashfault/renderer/renderer.h>
#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

namespace ashfault::editor {
Editor::Editor(std::shared_ptr<Window> window) : Application(window) {}

Editor::~Editor() {}

void Editor::run() {
  Renderer::init(m_Window);
  auto *ui_layer = new EditorUiLayer();
  m_LayerStack->push_layer(new RenderLayer());
  m_LayerStack->push_layer(new EditorLayer());
  m_LayerStack->push_overlay(ui_layer);

  m_Window->set_key_callback([&](Window &, int key, int, int action, int) {
    KeyPressEvent ev(key, action);
    m_LayerStack->on_event(ev);
  });


  SPDLOG_INFO("Editor startup finished");
  while (!m_Window->should_close()) {
    if (!Renderer::start_frame()) {
      SPDLOG_WARN("Couldn't start frame");
      continue;
    }

    m_LayerStack->on_update(1000.0f / 60.0f);
    m_LayerStack->on_render();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    m_LayerStack->on_imgui_render();
    ImGui::EndFrame();
    ImGui::Render();
    auto draw_data = ImGui::GetDrawData();
    Renderer::submit_imgui_data(draw_data);
    Renderer::end_frame();
    Renderer::submit_and_wait();
    m_Window->poll_events();
  }
  Renderer::shutdown();
}
} // namespace ashfault::editor
