#include <spdlog/spdlog.h>
#include <ashfault/editor/ui_layer.h>
#include <imgui.h>

namespace ashfault {
EditorUiLayer::EditorUiLayer() : Layer() {}
EditorUiLayer::~EditorUiLayer() {}

void EditorUiLayer::on_attach() {
  SPDLOG_INFO("Editor UI layer attach");
}

void EditorUiLayer::on_update(float dt) {}

void EditorUiLayer::on_render() {}

void EditorUiLayer::on_event(Event &event) {}

void EditorUiLayer::on_imgui_render() {
  ImGui::ShowDemoWindow();
}
} // namespace ashfault
