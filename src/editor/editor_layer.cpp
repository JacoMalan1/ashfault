#include <ashfault/core/component/mesh.h>
#include <ashfault/core/component/tag.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/event/key_press.h>
#include <ashfault/core/event/mouse_drag.h>
#include <ashfault/core/event/mouse_scroll.h>
#include <ashfault/core/event/viewport_resize.h>
#include <ashfault/core/layer_stack.h>
#include <ashfault/core/texture.h>
#include <ashfault/editor/context.h>
#include <ashfault/editor/editor_layer.h>
#include <ashfault/editor/event/state_change.h>
#include <ashfault/renderer/renderer.h>

#include <ashfault/core/event_bus.hpp>
#include <glm/trigonometric.hpp>
#include <memory>

namespace ashfault {
EditorLayer::EditorLayer(EditorContext *context,
                         std::shared_ptr<AssetManager> asset_manager)
    : Layer(),
      m_PerspectiveCamera(std::make_shared<PerspectiveCamera>(
          glm::radians(90.0f), 1280.0f / 720.0f)),
      m_Context(context),
      m_AssetManager(asset_manager),
      m_RuntimeState(State::Edit) {}

EditorLayer::~EditorLayer() {}

void EditorLayer::on_attach(LayerStack *) {
  std::uint32_t white = 0xffffffff;
  Renderer::upload_texture(reinterpret_cast<const char *>(&white), 1, 1);
  auto tex =
      m_AssetManager->load<Texture>("rock_wall", "rock_wall_16_diff_8k.jpg");
  auto norm = m_AssetManager->load<Texture>("rock_wall_norm",
                                            "rock_wall_16_nor_gl_8k.png");
  m_PerspectiveCamera->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
  m_Context->perspective_camera = m_PerspectiveCamera.get();

  m_ActiveScene = std::make_unique<Scene>();
  m_Context->active_scene = m_ActiveScene.get();

  MeshComponent mesh{};
  auto entity = m_ActiveScene->create_entity();
  mesh.mesh = m_AssetManager->load<Mesh>("./monkey.obj", "./monkey.obj");
  mesh.material = {
      .albedo_texture_index = static_cast<int>(tex.get()->index()),
      .normal_texture_index = static_cast<int>(norm.get()->index())};
  m_ActiveScene->component_registry().add_component(entity, mesh);

  EventBus<StateChangeEvent>::get().subscribe(
      [&](const StateChangeEvent &ev) { m_RuntimeState = ev.state(); });
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

    if (ev.button() == MouseDragEvent::Right) {
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

    m_PerspectiveCamera->zoom(-ev.delta() * 0.15);
  });
}

void EditorLayer::on_render() {
  Renderer::begin_scene(*m_PerspectiveCamera);
  m_ActiveScene->draw_all();
  Renderer::end_scene();
}
}  // namespace ashfault
