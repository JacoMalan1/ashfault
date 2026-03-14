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
  m_LayerStack->push_layer(new RenderLayer());
  m_LayerStack->push_layer(new EditorLayer());
  m_LayerStack->push_overlay(new EditorUiLayer());

  m_Window->set_key_callback([&](Window &, int key, int, int action, int) {
    KeyPressEvent ev(key, action);
    m_LayerStack->on_event(ev);
  });

  auto viewport_target = Renderer::create_render_target();

  while (!m_Window->should_close()) {
    Renderer::start_frame();
    Renderer::push_render_target(viewport_target);
    m_Window->poll_events();
    m_LayerStack->on_update(1000.0f / 60.0f);
    m_LayerStack->on_render();

    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
    m_LayerStack->on_imgui_render();
    ImGui::EndFrame();

    auto draw_data = ImGui::GetDrawData();
    Renderer::submit_imgui_data(draw_data);
    Renderer::end_frame();
    Renderer::submit_and_wait();
  }
}
} // namespace ashfault::editor
