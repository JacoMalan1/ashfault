#include <ashfault/core/event/key_press.h>
#include <ashfault/core/event/viewport_resize.h>
#include <ashfault/core/layer_stack.h>
#include <ashfault/editor/editor_layer.h>
#include <ashfault/renderer/renderer.h>

#include <glm/trigonometric.hpp>
#include <memory>

namespace ashfault {
EditorLayer::EditorLayer()
    : Layer(),
      m_PerspectiveCamera(std::make_shared<PerspectiveCamera>(
          glm::radians(90.0f), 1280.0f / 720.0f)) {}

EditorLayer::~EditorLayer() {}

void EditorLayer::on_attach(LayerStack*) {
  m_Mesh = Mesh::load_from_file("monkey.obj");
}

void EditorLayer::on_detach() {}

void EditorLayer::on_update(float dt) {}

void EditorLayer::on_event(Event& event) {
  Dispatcher dispatcher{};
  dispatcher.dispatch<ViewportResizeEvent>(event, [&](ViewportResizeEvent& ev) {
    m_PerspectiveCamera = std::make_shared<PerspectiveCamera>(
        glm::radians(90.0f),
        static_cast<float>(ev.width()) / static_cast<float>(ev.height()));
  });
}

void EditorLayer::on_render() {
  Renderer::begin_scene(*m_PerspectiveCamera);
  Renderer::submit_mesh(*m_Mesh);
  Renderer::end_scene();
}
}  // namespace ashfault
