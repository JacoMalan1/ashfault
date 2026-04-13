#include <ashfault/core/asset/mesh_loader.h>
#include <ashfault/core/asset/script_loader.h>
#include <ashfault/core/asset/texture_loader.h>
#include <ashfault/core/event/key_press.h>
#include <ashfault/core/layer/render_layer.h>
#include <ashfault/core/texture.h>
#include <ashfault/editor/editor.h>
#include <ashfault/editor/editor_layer.h>
#include <ashfault/editor/event/state_change.h>
#include <ashfault/editor/state.h>
#include <ashfault/editor/ui_layer.h>
#include <ashfault/renderer/renderer.h>
#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <spdlog/spdlog.h>

#include <ashfault/core/timer.hpp>
#include <ashfault/core/input.h>
#include <memory>

#include <ashfault/core/layer/script_layer.hpp>
#include <ashfault/core/event_bus.hpp>

using namespace std::chrono_literals;

namespace ashfault::editor {
Editor::Editor(std::shared_ptr<Window> window) : Application(window) {}

void Editor::run() {
  Renderer::init(m_Window);
  Input::init(m_Window);
  m_AssetManager->register_loader<Mesh>(std::make_shared<MeshLoader>());
  m_AssetManager->register_loader<Script>(std::make_shared<ScriptLoader>());
  m_AssetManager->register_loader<Texture>(std::make_shared<TextureLoader>());

  EditorContext context{};
  context.active_scene = nullptr;

  auto *ui_layer = new EditorUiLayer(&context, m_AssetManager);
  auto *script_layer = new ScriptLayer(m_AssetManager, &context);
  script_layer->set_enabled(false);

  m_LayerStack->push_layer(new RenderLayer());
  m_LayerStack->push_layer(script_layer);
  m_LayerStack->push_layer(new EditorLayer(&context, m_AssetManager));
  m_LayerStack->push_overlay(ui_layer);

  EventBus<StateChangeEvent>::get().subscribe([&](const StateChangeEvent &ev) {
    switch (ev.state()) {
      case State::Play:
        script_layer->set_enabled(true);
        break;
      case State::Edit:
        script_layer->set_enabled(false);
        break;
    }
  });

  SPDLOG_INFO("Editor startup finished");
  Timer<std::chrono::high_resolution_clock> timer{};
  timer.start();
  while (!m_Window->should_close()) {
    if (!Renderer::start_frame()) {
      SPDLOG_WARN("Couldn't start frame");
      continue;
    }

    float delta = timer.reset();
    m_LayerStack->on_update(delta / 1000.0f);
    m_LayerStack->on_render();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    m_LayerStack->on_imgui_render();
    ImGui::EndFrame();
    ImGui::Render();
    auto draw_data = ImGui::GetDrawData();
    Renderer::submit_imgui_data(draw_data);
    Renderer::end_frame();
    Renderer::submit_and_wait();
    m_Window->poll_events();
  }
  delete m_LayerStack;
  m_AssetManager->destroy();
  Renderer::shutdown();
}
}  // namespace ashfault::editor
