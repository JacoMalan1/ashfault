#include <ashfault/core/component/script.h>
#include <ashfault/core/component/light.h>
#include <ashfault/core/event/mouse_drag.h>
#include <ashfault/core/event/mouse_scroll.h>
#include <ashfault/core/event/scene_start.h>
#include <ashfault/core/event/viewport_resize.h>
#include <ashfault/editor/context.h>
#include <ashfault/editor/event/state_change.h>
#include <ashfault/editor/ui_layer.h>
#include <ashfault/renderer/renderer.h>
#include <ashfault/renderer/swapchain.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <spdlog/common.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/fmt/bundled/base.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/callback_sink.h>
#include <vulkan/vulkan_core.h>
#include <algorithm>
#include <ashfault/core/event_bus.hpp>
#include <filesystem>
#include <memory>
#include <ImGuizmo.h>
#include <ashfault/core/component/mesh.h>
#include <ashfault/core/component/tag.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/mesh.h>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <imgui_internal.h>

namespace ashfault {
EditorUiLayer::EditorUiLayer(EditorContext *context,
                             std::shared_ptr<AssetManager> asset_manager)
    : Layer(),
      m_ViewportTarget(Renderer::create_render_target(1, 1, false, false)),
      m_ViewportTextures(),
      m_UpdateViewport(true),
      m_EditorContext(context),
      m_AssetManager(asset_manager),
      m_RuntimeState(State::Edit) {}

EditorUiLayer::~EditorUiLayer() {
  m_ViewportTarget->destroy();
  for (auto tex : m_ViewportTextures) {
    ImGui_ImplVulkan_RemoveTexture(tex);
  }
  vkDestroySampler(Renderer::vulkan_backend().device(), m_ViewportSampler, nullptr);
}

void EditorUiLayer::on_attach(LayerStack *stack) {
  SPDLOG_INFO("Editor UI layer attach");

  ImVec4 *colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.60f, 0.59f, 0.10f, 0.54f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.60f, 0.59f, 0.10f, 0.40f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.60f, 0.59f, 0.10f, 0.67f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.60f, 0.59f, 0.11f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.60f, 0.59f, 0.10f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.60f, 0.59f, 0.10f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.59f, 0.10f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.60f, 0.59f, 0.10f, 0.40f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.60f, 0.59f, 0.10f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.72f, 0.73f, 0.15f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.60f, 0.59f, 0.10f, 0.31f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.60f, 0.59f, 0.10f, 0.80f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.60f, 0.59f, 0.10f, 1.00f);
  colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.59f, 0.10f, 0.78f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.60f, 0.59f, 0.10f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.60f, 0.59f, 0.10f, 0.20f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.60f, 0.59f, 0.10f, 0.67f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.60f, 0.59f, 0.10f, 0.95f);
  colors[ImGuiCol_InputTextCursor] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.60f, 0.59f, 0.10f, 0.80f);
  colors[ImGuiCol_Tab] = ImVec4(0.60f, 0.59f, 0.10f, 0.86f);
  colors[ImGuiCol_TabSelected] = ImVec4(0.60f, 0.59f, 0.10f, 1.00f);
  colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.60f, 0.59f, 0.10f, 1.00f);
  colors[ImGuiCol_TabDimmed] = ImVec4(0.13f, 0.12f, 0.01f, 0.97f);
  colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.26f, 0.26f, 0.03f, 0.97f);
  colors[ImGuiCol_TabDimmedSelectedOverline] =
      ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
  colors[ImGuiCol_DockingPreview] = ImVec4(0.59f, 0.60f, 0.10f, 0.70f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.80f, 0.14f, 0.11f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.98f, 0.74f, 0.18f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.84f, 0.60f, 0.13f, 1.00f);
  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
  colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
  colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
  colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
  colors[ImGuiCol_TextLink] = ImVec4(0.60f, 0.59f, 0.10f, 1.00f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.60f, 0.59f, 0.10f, 0.35f);
  colors[ImGuiCol_TreeLines] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(0.72f, 0.73f, 0.15f, 0.90f);
  colors[ImGuiCol_DragDropTargetBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_UnsavedMarker] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_NavCursor] = ImVec4(0.59f, 0.59f, 0.10f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

  auto sink = std::make_shared<spdlog::sinks::callback_sink_st>(
      [&](const spdlog::details::log_msg &msg) {
        char *buf = new char[msg.payload.size() + 1];
        std::memcpy(buf, msg.payload.data(), msg.payload.size());
        buf[msg.payload.size()] = 0;
        m_LogLines.push_back(std::make_pair(msg.level, std::string(buf)));
        delete[] buf;
      });
  spdlog::default_logger()->sinks().push_back(sink);

  auto &io = ImGui::GetIO();
  auto *roboto = io.Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", 18.0f);
  io.Fonts->Build();
  io.IniFilename = nullptr;
  io.FontDefault = roboto;

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
    m_ViewportTarget->destroy();
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

void EditorUiLayer::build_ui_skeleton() {
  ImGuiID dockspace_id = ImGui::GetID("Main Dockspace");
  ImGuiViewport *viewport = ImGui::GetMainViewport();
  if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

    ImGuiID dock_id_bottom = 0;
    ImGuiID dock_id_main = dockspace_id;
    ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Down, 0.20f,
                                &dock_id_bottom, &dock_id_main);

    ImGuiID dock_id_left = 0;
    ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.30f,
                                &dock_id_left, &dock_id_main);

    ImGuiID dock_id_left_bottom = 0;
    ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.5f,
                                &dock_id_left_bottom, &dock_id_left);

    ImGuiID dock_id_right = 0;
    ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.40f,
                                &dock_id_right, &dock_id_main);

    ImGuiID dock_id_right_top = 0;
    ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Up, 0.05f,
                                &dock_id_right_top, &dock_id_right);
    ImGuiDockNode *node = ImGui::DockBuilderGetNode(dock_id_right_top);
    node->SetLocalFlags(ImGuiDockNodeFlags_NoTabBar);

    ImGui::DockBuilderDockWindow("Scene Graph", dock_id_left);
    ImGui::DockBuilderDockWindow("File Browser", dock_id_left_bottom);
    ImGui::DockBuilderDockWindow("Components", dock_id_right);
    ImGui::DockBuilderDockWindow("Toolbar", dock_id_right_top);
    ImGui::DockBuilderDockWindow("Viewport", dock_id_main);
    ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
    ImGui::DockBuilderFinish(dockspace_id);
  }

  ImGui::DockSpaceOverViewport(dockspace_id, viewport,
                               ImGuiDockNodeFlags_PassthruCentralNode);
}

void render_directory(const std::filesystem::path &path,
                      bool default_open = false) {
  auto begin =
      std::filesystem::begin(std::filesystem::directory_iterator(path));
  auto end = std::filesystem::end(std::filesystem::directory_iterator(path));

  ImGuiTreeNodeFlags flags = 0;
  if (default_open) {
    flags |= ImGuiTreeNodeFlags_DefaultOpen;
  }
  if (ImGui::TreeNodeEx(reinterpret_cast<const char *>(path.filename().c_str()),
                        flags)) {
    for (auto it = begin; it != end; it++) {
      auto path = it->path();
      if (path.has_filename() && path.filename().string().starts_with(".")) {
        continue;
      }
      if (it->is_directory()) {
        render_directory(path);
      } else if (path.has_extension() && path.extension().string() == ".obj") {
        if (ImGui::TreeNodeEx(
                reinterpret_cast<const char *>(path.filename().c_str()),
                ImGuiTreeNodeFlags_Leaf)) {
          if (ImGui::BeginDragDropSource()) {
            auto str = it->path().string();
            ImGui::SetDragDropPayload("obj", str.c_str(),
                                      std::strlen(str.c_str()) + 1);
            ImGui::EndDragDropSource();
          }
          ImGui::TreePop();
        }
      } else if (it->path().has_extension() &&
                 it->path().extension().string() == ".lua") {
        if (ImGui::TreeNodeEx(
                reinterpret_cast<const char *>(path.filename().c_str()),
                ImGuiTreeNodeFlags_Leaf)) {
          if (ImGui::BeginDragDropSource()) {
            auto str = path.string();
            ImGui::SetDragDropPayload("lua", str.c_str(),
                                      std::strlen(str.c_str()) + 1);
            ImGui::EndDragDropSource();
          }
          ImGui::TreePop();
        }
      }
    }
    ImGui::TreePop();
  }
}

void EditorUiLayer::render_file_browser() {
  ImGui::Begin("File Browser");
  render_directory(".", true);
  ImGui::End();
}

void EditorUiLayer::render_scene_window() {
  ImGui::Begin("Scene Graph");
  if (ImGui::TreeNode("Scene")) {
    if (m_EditorContext->active_scene) {
      if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Create Entity")) {
          m_EditorContext->active_scene->create_entity();
        }
        ImGui::EndPopup();
      }
      for (auto &e : m_EditorContext->active_scene->entities()) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
        auto tag = m_EditorContext->active_scene->component_registry()
                       .get_component<TagComponent>(e);
        std::string name = "Entity";
        if (tag.has_value()) {
          name = tag.value()->tag;
        }

        if (m_EditorContext->selected_entity.has_value() &&
            m_EditorContext->selected_entity.value() == e) {
          flags |= ImGuiTreeNodeFlags_Selected;
        }

        if (ImGui::TreeNodeEx((void *)(std::uint64_t)e.handle(), flags, "%s",
                              name.c_str())) {
          if (ImGui::IsItemClicked()) {
            m_EditorContext->selected_entity = e;
          }
          if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
              m_EditorContext->active_scene->delete_entity(e);
              m_EditorContext->selected_entity = {};
            }
            ImGui::EndPopup();
          }
          ImGui::TreePop();
        }
      }
    }
    ImGui::TreePop();
  }
  ImGui::End();
}

void EditorUiLayer::render_console_window() {
  ImGui::Begin("Console");
  for (const auto &[level, msg] : m_LogLines) {
    spdlog::string_view_t level_view = spdlog::level::to_string_view(level);
    ImGui::Text("%s: %s", level_view.data(), msg.c_str());
  }
  ImGui::ScrollToItem();
  ImGui::End();
}

void EditorUiLayer::render_component_window() {
  ImGui::Begin("Components", nullptr, ImGuiWindowFlags_MenuBar);
  auto *scene = m_EditorContext->active_scene;
  if (m_EditorContext->selected_entity.has_value() && scene) {
    auto entity = m_EditorContext->selected_entity.value();
    auto tag = scene->component_registry().get_component<TagComponent>(entity);
    auto transform =
        scene->component_registry().get_component<TransformComponent>(entity);
    auto mesh =
        scene->component_registry().get_component<MeshComponent>(entity);
    auto script =
        scene->component_registry().get_component<ScriptComponent>(entity);
    auto directional_light =
        scene->component_registry().get_component<DirectionalLightComponent>(
            entity);
    auto point_light =
        scene->component_registry().get_component<PointLightComponent>(entity);
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("Add Component")) {
        if (!tag.has_value() && ImGui::MenuItem("Tag")) {
          TagComponent tag{.tag = "Entity"};
          scene->component_registry().add_component(entity, tag);
        }

        if (!transform.has_value() && ImGui::MenuItem("Transform")) {
          TransformComponent transform{.position = glm::vec3(0),
                                       .rotation = glm::vec3(0),
                                       .scale = glm::vec3(1)};
          scene->component_registry().add_component(entity, transform);
        }

        if (!mesh.has_value() && ImGui::MenuItem("Mesh")) {
          MeshComponent mesh{
              .mesh = m_AssetManager->load<Mesh>("monkey", "monkey.obj")};
          scene->component_registry().add_component(entity, mesh);
        }
        if (!script.has_value() && ImGui::MenuItem("Script")) {
          ScriptComponent script{
              .script = m_AssetManager->load<Script>("script", "script.lua")};
          scene->component_registry().add_component(entity, script);
        }
        if (!directional_light.has_value() &&
            ImGui::MenuItem("Directional Light")) {
          DirectionalLightComponent light{
              .direction = glm::vec3(0.0f, 0.0f, 1.0f),
              .color = glm::vec3(1.0f)};
          scene->component_registry().add_component(entity, light);
        }
        if (!point_light.has_value() && ImGui::MenuItem("Point Light")) {
          PointLightComponent light{.position = glm::vec3(0.0f),
                                    .color = glm::vec3(1.0f)};

          scene->component_registry().add_component(entity, light);
          if (!transform.has_value()) {
            TransformComponent transform{.position = glm::vec3(0),
                                         .rotation = glm::vec3(0),
                                         .scale = glm::vec3(1)};
            scene->component_registry().add_component(entity, transform);
          }
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    if (tag.has_value()) {
      ImGui::SeparatorText("Tag");
      char buf[1024];
      std::strcpy(buf, tag.value()->tag.c_str());
      ImGui::InputText("Name", buf, 1024);
      tag.value()->tag = buf;

      if (ImGui::Button("Delete Component##Tag")) {
        scene->component_registry().remove_component<TagComponent>(entity);
      }
    }

    if (transform.has_value()) {
      ImGui::SeparatorText("Transform");
      transform.value()->rotation = glm::degrees(transform.value()->rotation);
      ImGui::DragFloat3("Position",
                        reinterpret_cast<float *>(&transform.value()->position),
                        0.1f, -100.0f, 100.0f);

      ImGui::DragFloat3("Rotation",
                        reinterpret_cast<float *>(&transform.value()->rotation),
                        0.1f, -100.0f, 100.0f);
      ImGui::DragFloat3("Scale",
                        reinterpret_cast<float *>(&transform.value()->scale),
                        0.1f, -100.0f, 100.0f);

      transform.value()->rotation = glm::radians(transform.value()->rotation);
      if (ImGui::Button("Delete Component##Transform")) {
        scene->component_registry().remove_component<TransformComponent>(
            entity);
      }
    }

    if (mesh.has_value()) {
      ImGui::SeparatorText("Mesh");
      ImGui::BeginDisabled();
      std::string id = mesh.value()->mesh.id();
      ImGui::InputText("File##Mesh", id.data(), id.size() + 1);
      if (ImGui::BeginDragDropTarget()) {
        if (auto payload = ImGui::AcceptDragDropPayload("obj")) {
          std::string str;
          str.resize(payload->DataSize);
          std::strcpy(str.data(), static_cast<char *>(payload->Data));
          mesh.value()->mesh = m_AssetManager->load<Mesh>(str, str);
        }
        ImGui::EndDragDropTarget();
      }
      ImGui::EndDisabled();
      if (ImGui::Button("Delete Component##Mesh")) {
        scene->component_registry().remove_component<MeshComponent>(entity);
      }
    }

    if (script.has_value()) {
      ImGui::SeparatorText("Script");
      ImGui::BeginDisabled();
      std::string id = script.value()->script.id();
      ImGui::InputText("File##Script", id.data(), id.size() + 1);
      if (ImGui::BeginDragDropTarget()) {
        if (auto payload = ImGui::AcceptDragDropPayload("lua")) {
          std::string str;
          str.resize(payload->DataSize);
          std::strcpy(str.data(), static_cast<char *>(payload->Data));
          script.value()->script = m_AssetManager->load<Script>(str, str);
        }
        ImGui::EndDragDropTarget();
      }
      ImGui::EndDisabled();
      if (ImGui::Button("Reload")) {
        SPDLOG_WARN("Hot-reloading script '{}'", script.value()->script.path());
        m_AssetManager->reload<Script>(id);
      }
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("Hot-reload this script");
      }
      if (ImGui::Button("Delete Component##Script")) {
        scene->component_registry().remove_component<ScriptComponent>(entity);
      }
    }

    if (directional_light.has_value()) {
      ImGui::SeparatorText("Directional Light");
      ImGui::DragFloat3(
          "Direction##directional_light",
          reinterpret_cast<float *>(&directional_light.value()->direction),
          0.01f, -1.0f, 1.0f);
      ImGui::ColorPicker3(
          "Color##directional_light",
          reinterpret_cast<float *>(&directional_light.value()->color),
          ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_Float |
              ImGuiColorEditFlags_InputRGB);
      if (ImGui::Button("Delete Component##directional_light")) {
        scene->component_registry().remove_component<DirectionalLightComponent>(
            entity);
      }
    }

    if (point_light.has_value()) {
      ImGui::SeparatorText("Point Light");
      ImGui::ColorPicker3(
          "Color##point_light",
          reinterpret_cast<float *>(&point_light.value()->color),
          ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_Float |
              ImGuiColorEditFlags_InputRGB);
      if (ImGui::Button("Delete Component##point_light")) {
        scene->component_registry().remove_component<PointLightComponent>(
            entity);
      }
    }
  }
  ImGui::End();
}

void EditorUiLayer::render_toolbar() {
  ImGui::Begin("Toolbar", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoScrollWithMouse |
                   ImGuiWindowFlags_NoTitleBar);
  if (m_RuntimeState == State::Edit) {
    if (ImGui::Button("Play")) {
      StateChangeEvent ev(State::Play);
      EventBus<StateChangeEvent>::get().dispatch(ev);
      m_RuntimeState = State::Play;
      if (m_LayerStack && m_EditorContext->active_scene) {
        SceneStartEvent ev(m_EditorContext->active_scene);
        m_LayerStack->on_event(ev);
      }
    }
  } else if (m_RuntimeState == State::Play) {
    if (ImGui::Button("Edit")) {
      StateChangeEvent ev(State::Edit);
      EventBus<StateChangeEvent>::get().dispatch(ev);
      m_RuntimeState = State::Edit;
    }
  }
  ImGui::End();
}

void EditorUiLayer::on_imgui_render() {
  Renderer::pop_render_target();
  build_ui_skeleton();
  render_scene_window();
  render_component_window();
  render_console_window();
  render_file_browser();
  render_toolbar();

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Exit")) {
        std::exit(0);
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("Viewport");
  auto size = ImGui::GetContentRegionAvail();
  if (m_PreviousViewportSize.has_value() &&
      (m_PreviousViewportSize.value().x != size.x ||
       m_PreviousViewportSize.value().y != size.y)) {
    m_UpdateViewport = true;
  }
  m_PreviousViewportSize = size;
  ImGui::Image(m_ViewportTextures[Renderer::swapchain_image_index()], size);

  ImVec2 combo_pos = ImGui::GetWindowPos();
  combo_pos.x += ImGui::GetWindowWidth() - 250;
  combo_pos.y += 40;
  ImGui::SetCursorScreenPos(combo_pos);
  ImGui::SetNextItemWidth(190.0f);
  ImGui::SetNextItemAllowOverlap();
  const char *items[] = {"Translate", "Rotate", "Scale"};
  static int currentItem = 0;
  ImGui::Combo("Mode", &currentItem, items, IM_ARRAYSIZE(items));

  ImGuizmo::OPERATION operation;
  switch (currentItem) {
    case 0:
      operation = ImGuizmo::OPERATION::TRANSLATE;
      break;
    case 1:
      operation = ImGuizmo::OPERATION::ROTATE;
      break;
    case 2:
      operation = ImGuizmo::OPERATION::SCALE;
      break;
  }

  ImGuizmo::SetOrthographic(false);
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y,
                    ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
  auto view_mat = m_EditorContext->perspective_camera->view();
  ImVec2 gizmo_pos = ImGui::GetWindowPos();
  gizmo_pos.x += 10.0f;
  gizmo_pos.y += 10.0f;
  ImGuizmo::ViewManipulate(reinterpret_cast<float *>(&view_mat),
                           m_EditorContext->perspective_camera->radius(),
                           gizmo_pos, ImVec2(150, 150), 0);
  m_EditorContext->perspective_camera->set_orientation(
      glm::quat_cast(glm::inverse(view_mat)));

  bool manipulating = false;
  if (m_EditorContext->active_scene &&
      m_EditorContext->selected_entity.has_value()) {
    auto e = m_EditorContext->selected_entity.value();
    auto transform = m_EditorContext->active_scene->component_registry()
                         .get_component<TransformComponent>(e);
    if (transform.has_value()) {
      glm::vec3 translation = transform.value()->position,
                rotation = glm::degrees(transform.value()->rotation),
                scale = transform.value()->scale;
      glm::mat4 model_mat;
      ImGuizmo::RecomposeMatrixFromComponents(
          reinterpret_cast<float *>(&translation),
          reinterpret_cast<float *>(&rotation),
          reinterpret_cast<float *>(&scale),
          reinterpret_cast<float *>(&model_mat));
      auto proj = m_EditorContext->perspective_camera->projection();
      proj[1][1] *= -1.0f;
      ImGuizmo::Manipulate(reinterpret_cast<float *>(&view_mat),
                           reinterpret_cast<float *>(&proj), operation,
                           ImGuizmo::MODE::WORLD,
                           reinterpret_cast<float *>(&model_mat));
      ImGuizmo::DecomposeMatrixToComponents(
          reinterpret_cast<float *>(&model_mat),
          reinterpret_cast<float *>(&translation),
          reinterpret_cast<float *>(&rotation),
          reinterpret_cast<float *>(&scale));
      transform.value()->position = translation;
      transform.value()->scale = scale;
    }
  }
  manipulating = ImGuizmo::IsUsingAny();

  if (!manipulating) {
    if (ImGui::IsWindowHovered() &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
      auto delta = ImGui::GetIO().MouseDelta;
      MouseDragEvent ev(MouseDragEvent::Right, delta.x, delta.y);
      if (m_LayerStack) {
        m_LayerStack->on_event(ev);
      }
    }

    if (ImGui::IsWindowHovered() &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
      auto delta = ImGui::GetIO().MouseDelta;
      MouseDragEvent ev(MouseDragEvent::Middle, delta.x, delta.y);
      if (m_LayerStack) {
        m_LayerStack->on_event(ev);
      }
    }

    if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheel != 0) {
      auto delta = ImGui::GetIO().MouseWheel;
      MouseScrollEvent ev(delta);
      if (m_LayerStack) {
        m_LayerStack->on_event(ev);
      }
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
