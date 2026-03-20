#include <ashfault/core/event/mouse_drag.h>
#include <ashfault/core/event/viewport_resize.h>
#include <ashfault/editor/ui_layer.h>
#include <ashfault/renderer/renderer.h>
#include <ashfault/renderer/swapchain.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <spdlog/fmt/bundled/base.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>

#include "ashfault/core/event/mouse_scroll.h"

namespace ashfault {
EditorUiLayer::EditorUiLayer()
    : Layer(),
      m_ViewportTarget(Renderer::create_render_target(1, 1, false, false)),
      m_ViewportTextures(),
      m_UpdateViewport(true) {}

EditorUiLayer::~EditorUiLayer() {}

void EditorUiLayer::on_attach(LayerStack *stack) {
  SPDLOG_INFO("Editor UI layer attach");
  auto &renderer = Renderer::vulkan_backend();
  m_LayerStack = stack;
  m_ViewportSampler = renderer.create_sampler();

  m_ViewportTextures.resize(renderer.swapchain()->image_count());
  for (std::size_t i = 0; i < renderer.swapchain()->image_count(); i++) {
    m_ViewportTextures[i] = ImGui_ImplVulkan_AddTexture(
        m_ViewportSampler, m_ViewportTarget->image_view(i),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
}

void EditorUiLayer::on_detach() { m_LayerStack = nullptr; }

void EditorUiLayer::on_update(float dt) {
  if (m_UpdateViewport) {
    ImVec2 dims = m_PreviousViewportSize.value_or(ImVec2(1, 1));
    m_ViewportTarget = Renderer::create_render_target(
        std::clamp<std::uint32_t>(dims.x, 1, 8192),
        std::clamp<std::uint32_t>(dims.y, 1, 8192), false, false);
    recreate_textures();
    m_UpdateViewport = false;
    if (m_LayerStack) {
      ViewportResizeEvent ev(dims.x, dims.y);
      m_LayerStack->on_event(ev);
    }
  }
}

void EditorUiLayer::on_render() {
  Renderer::push_render_target(m_ViewportTarget);
}

void EditorUiLayer::on_event(Event &event) {}

void EditorUiLayer::on_imgui_render() {
  Renderer::pop_render_target();
  ImGui::DockSpaceOverViewport();
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("Scene");
  auto size = ImGui::GetContentRegionAvail();
  if (m_PreviousViewportSize.has_value() &&
      (m_PreviousViewportSize.value().x != size.x ||
       m_PreviousViewportSize.value().y != size.y)) {
    m_UpdateViewport = true;
  }
  m_PreviousViewportSize = size;
  ImGui::Image(m_ViewportTextures[Renderer::swapchain_image_index()], size);

  if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
    auto delta = ImGui::GetIO().MouseDelta;
    MouseDragEvent ev(MouseDragEvent::Left, delta.x, delta.y);
    if (m_LayerStack) {
      m_LayerStack->on_event(ev);
    }
  }

  if (ImGui::IsItemHovered() &&
      ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
    auto delta = ImGui::GetIO().MouseDelta;
    MouseDragEvent ev(MouseDragEvent::Middle, delta.x, delta.y);
    if (m_LayerStack) {
      m_LayerStack->on_event(ev);
    }
  }

  if (ImGui::IsItemHovered() && ImGui::GetIO().MouseWheel != 0) {
    auto delta = ImGui::GetIO().MouseWheel;
    MouseScrollEvent ev(delta);
    if (m_LayerStack) {
      m_LayerStack->on_event(ev);
    }
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

void EditorUiLayer::recreate_textures() {
  auto &renderer = Renderer::vulkan_backend();
  for (std::size_t i = 0; i < renderer.swapchain()->image_count(); i++) {
    ImGui_ImplVulkan_RemoveTexture(m_ViewportTextures[i]);
    m_ViewportTextures[i] = ImGui_ImplVulkan_AddTexture(
        m_ViewportSampler, m_ViewportTarget->image_view(i),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
}
}  // namespace ashfault
