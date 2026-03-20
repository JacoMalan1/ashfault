#include <ashfault/core/event/key_press.h>
#include <ashfault/core/event/mouse_drag.h>
#include <ashfault/core/event/mouse_scroll.h>
#include <ashfault/core/event/viewport_resize.h>
#include <ashfault/core/layer_stack.h>
#include <ashfault/editor/editor_layer.h>
#include <ashfault/renderer/renderer.h>

#include <glm/trigonometric.hpp>
#include <memory>

#include "ashfault/core/component/mesh.h"
#include "ashfault/core/component/transform.h"

namespace ashfault {
EditorLayer::EditorLayer()
    : Layer(),
      m_PerspectiveCamera(std::make_shared<PerspectiveCamera>(
          glm::radians(90.0f), 1280.0f / 720.0f)) {}

EditorLayer::~EditorLayer() {}

void EditorLayer::on_attach(LayerStack *) {
  m_Mesh = Mesh::load_from_file("monkey.obj");
  m_PerspectiveCamera->set_position(glm::vec3(0.0f, 0.0f, 0.0f));

  m_ActiveScene = std::make_shared<Scene>();
  auto e = m_ActiveScene->create_entity();
  MeshComponent mesh_component = {.mesh = m_Mesh};
  TransformComponent transform = {.position = glm::vec3(0.0f),
                                  .rotation = glm::vec3(0.0f, 0.0f, 0.0f),
                                  .scale = glm::vec3(1.0f)};

  m_ActiveScene->component_registry().add_component(e, mesh_component);
  m_ActiveScene->component_registry().add_component(e, transform);
}

void EditorLayer::on_detach() {}

void EditorLayer::on_update(float dt) {}

void EditorLayer::on_event(Event &event) {
  Dispatcher dispatcher{};
  dispatcher.dispatch<ViewportResizeEvent>(event, [&](ViewportResizeEvent &ev) {
    ev.set_handled();
    m_PerspectiveCamera->set_aspect_ratio(ev.width(), ev.height());
  });

  dispatcher.dispatch<MouseDragEvent>(event, [&](MouseDragEvent &ev) {
    ev.set_handled();

    if (ev.button() == MouseDragEvent::Left) {
      auto rotation = glm::vec3(ev.delta_x(), ev.delta_y(), 0.0f) / 100.0f;
      m_PerspectiveCamera->set_rotation(rotation);
    }

    if (ev.button() == MouseDragEvent::Middle) {
      m_PerspectiveCamera->pan(glm::vec3(ev.delta_x(), ev.delta_y(), 0.0f) *
                               0.05f);
    }
  });

  dispatcher.dispatch<MouseScrollEvent>(event, [&](MouseScrollEvent &ev) {
    ev.set_handled();

    m_PerspectiveCamera->zoom(-ev.delta() * 0.08);
  });
}

void EditorLayer::on_render() {
  Renderer::begin_scene(*m_PerspectiveCamera);
  m_ActiveScene->draw_all();
  Renderer::end_scene();
}
}  // namespace ashfault
