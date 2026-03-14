#include <ashfault/renderer/vkrenderer.h>
#include <ashfault/core/event/key_press.h>
#include <ashfault/core/layer/render_layer.h>
#include <spdlog/spdlog.h>

namespace ashfault {
RenderLayer::RenderLayer() : Layer() {}

void RenderLayer::on_attach() {}
void RenderLayer::on_detach() {}

void RenderLayer::on_update(float) {
}

void RenderLayer::on_render() {}

void RenderLayer::on_event(Event &event) {
  Dispatcher dispatcher;
  dispatcher.dispatch<KeyPressEvent>(event, [](KeyPressEvent &ev) {
    SPDLOG_INFO("Key press event in Renderer: {}", ev.key_code());
    ev.set_handled();
  });
}
} // namespace ashfault
