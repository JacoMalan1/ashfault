#include "glm/ext/scalar_constants.hpp"
#include "glm/gtc/constants.hpp"
#include <CLSTL/shared_ptr.h>
#include <algorithm>
#include <ashfault/core/component/mesh.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/scene.h>
#include <ashfault/core/vertex.h>
#include <ashfault/core/window.h>
#include <ashfault/editor/camera.h>
#include <ashfault/editor/editor.h>
#include <ashfault/renderer/descriptor_set.h>
#include <ashfault/renderer/frame.h>
#include <ashfault/renderer/renderer.h>
#include <ashfault/renderer/swapchain.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>
#include <memory>
#include <mutex>
#include <spdlog/common.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/sinks/callback_sink.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include <ImGuizmo.h>

namespace ashfault::editor {

Editor::Editor(clstl::shared_ptr<Engine> engine,
               clstl::shared_ptr<Window> window)
    : Application(engine, window),
      m_ViewportSize({(std::uint32_t)1, (std::uint32_t)1}),
      m_CurrentWindowSize({1, 1}), m_ViewportResized(false), m_LogsLock(),
      m_Logs(), m_PipelineManager(clstl::make_unique<PipelineManager>()) {
  auto callback_sink = std::make_shared<spdlog::sinks::callback_sink_mt>(
      [&](const spdlog::details::log_msg &msg) {
        std::unique_lock<std::mutex> lck(m_LogsLock);
        m_Logs.push_back(std::make_pair<>(msg, msg.payload.data()));
      });
  callback_sink->set_level(spdlog::level::debug);
  spdlog::default_logger()->sinks().push_back(callback_sink);

  auto camera = std::make_shared<PerspectiveEditorCamera>(
      PerspectiveEditorCamera::builder()
          .fov(glm::half_pi<float>())
          .aspect_ratio(static_cast<float>(m_CurrentWindowSize.width) /
                        static_cast<float>(m_CurrentWindowSize.height))
          .build());
  this->m_CameraControls = std::make_unique<PerspectiveCameraControls>(camera);
  this->m_Camera = camera;
}

void Editor::build_pipelines() {
  VkVertexInputAttributeDescription desc{};
  desc.binding = 0;
  desc.offset = 0;
  desc.location = 0;
  desc.format = VK_FORMAT_R32G32B32_SFLOAT;

  VkVertexInputAttributeDescription normal{};
  normal.binding = 0;
  normal.location = 1;
  normal.offset = 3 * sizeof(float);
  normal.format = VK_FORMAT_R32G32B32_SFLOAT;

  clstl::vector<VkVertexInputAttributeDescription> descriptions;
  descriptions.push_back(desc);
  descriptions.push_back(normal);

  clstl::vector<clstl::shared_ptr<VulkanDescriptorSet>> dsets;
  dsets.push_back(this->m_DescriptorSet);

  auto pipeline =
      this->m_Engine->renderer()
          .create_graphics_pipeline()
          .vertex_shader(this->m_Engine->shader_manager()
                             .get_vertex_shader("blinn_phong")
                             .value())
          .fragment_shader(this->m_Engine->shader_manager()
                               .get_fragment_shader("blinn_phong")
                               .value())
          .input_attribute_descriptions(descriptions, sizeof(StaticVertex))
          .descriptor_sets(dsets)
          .push_constant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4))
          .build();
  this->m_Engine->pipeline_manager().add_graphics_pipeline("simple", pipeline);
}

void Editor::build_ui_skeleton() {
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
    ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 1.20f,
                                &dock_id_left, &dock_id_main);

    ImGuiID dock_id_right = 0;
    ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 1.20f,
                                &dock_id_right, &dock_id_main);

    ImGui::DockBuilderDockWindow("Scene", dock_id_left);
    ImGui::DockBuilderDockWindow("Components", dock_id_right);
    ImGui::DockBuilderDockWindow("Viewport", dock_id_main);
    ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
    ImGui::DockBuilderFinish(dockspace_id);
  }

  ImGui::DockSpaceOverViewport(dockspace_id, viewport,
                               ImGuiDockNodeFlags_PassthruCentralNode);
  ImGui::Begin("Scene");
  if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::TreeNodeEx("Triangle", ImGuiTreeNodeFlags_Leaf)) {
      ImGui::TreePop();
    }
    ImGui::TreePop();
  }
  ImGui::End();

  ImGui::Begin("Components", nullptr, ImGuiWindowFlags_MenuBar);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Add Component")) {
      ImGui::MenuItem("Transform");
      ImGui::MenuItem("Mesh");
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  this->m_CameraControls->render_controls();
  ImGui::End();

  ImGui::Begin("Console");
  std::for_each(
      m_Logs.begin(), m_Logs.end(),
      [](const std::pair<spdlog::details::log_msg, std::string> &msg) {
        auto level = spdlog::level::to_string_view(msg.first.level);
        ImVec4 color;
        switch (msg.first.level) {
        case spdlog::level::err:
          color = ImVec4(255.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 1.0f);
          break;
        case spdlog::level::info:
          color = ImVec4(47.0f / 255.0f, 255.0f / 255.0f, 20.0f / 255.0f, 1.0f);
          break;
        case spdlog::level::warn:
          color =
              ImVec4(255.0f / 255.0f, 251.0f / 255.0f, 20.0f / 255.0f, 1.0f);
          break;
        default:
          color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        }

        ImGui::TextColored(color, "[%s]: %s", level.data(), msg.second.c_str());
      });
  ImGui::End();
}

SubmitData Editor::render_ui(Frame &frame) {
  auto cmd = this->m_UiCommandBuffers[frame.current_frame()];
  VulkanRenderingAttachments attachments;
  attachments.build_color_attachment()
      .clear_color(0.0f, 0.0f, 0.0f, 1.0f)
      .target(this->m_Engine->renderer().swapchain()->image_view(
                  frame.image_index()),
              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  VkExtent2D swap_extent = m_Engine->renderer().swapchain()->swap_extent();
  VkRect2D render_area{};
  render_area.extent.width = swap_extent.width;
  render_area.extent.height = swap_extent.height;
  render_area.offset.x = 0;
  render_area.offset.y = 0;

  frame.begin_command_buffer(cmd);
  frame.insert_pipeline_barrier(
      cmd, this->m_Engine->renderer().swapchain()->image(frame.image_index()),
      VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_NONE,
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
  frame.insert_pipeline_barrier(
      cmd, this->m_ViewportImages[frame.image_index()].first,
      VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_NONE,
      VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
  frame.begin_rendering(cmd, attachments, render_area);
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGuizmo::BeginFrame();
  build_ui_skeleton();
  ImGui::Begin("Viewport");
  ImVec2 window_pos = ImGui::GetWindowPos();
  ImVec2 window_size = ImGui::GetWindowSize();
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetRect(window_pos.x, window_pos.y, window_size.x, window_size.y);
  glm::mat4 view_mat = this->m_Camera->view();
  view_mat = glm::scale(view_mat, glm::vec3(1.0f, 1.0f, -1.0f));

  auto size = ImGui::GetContentRegionAvail();
  if (size.x != m_ViewportSize[0] || size.y != m_ViewportSize[1]) {
    m_ViewportResized = true;
  }
  m_ViewportSize[0] = size.x;
  m_ViewportSize[1] = size.y;
  ImGui::Image(this->m_ImGuiViewportTextures[frame.image_index()], size);
  ImGuizmo::ViewManipulate(reinterpret_cast<float *>(&view_mat), 1.0f,
                           ImGui::GetWindowPos(), ImVec2(200.0f, 200.0f), 0);
  ImGuizmo::Enable(true);
  view_mat = glm::scale(view_mat, glm::vec3(1.0f, 1.0f, -1.0f));
  glm::vec3 translation, rotation, scale;
  ImGuizmo::DecomposeMatrixToComponents(reinterpret_cast<float *>(&view_mat),
                                        reinterpret_cast<float *>(&translation),
                                        reinterpret_cast<float *>(&rotation),
                                        reinterpret_cast<float *>(&scale));
  if (ImGui::IsItemHovered() &&
      ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
    this->m_Camera->rotate(glm::vec3(ImGui::GetIO().MouseDelta.y * 0.01,
                                     -ImGui::GetIO().MouseDelta.x * 0.01,
                                     0.0f));
  }

  if (ImGui::IsItemHovered() &&
      ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
    auto delta = ImGui::GetIO().MouseDelta;
    this->m_Camera->move(0.01f * glm::vec3(-delta.x, -delta.y, 0.0f));
  }

  if (ImGui::IsItemHovered()) {
    this->m_Camera->move(0.08f *
                         glm::vec3(0.0f, 0.0f, -ImGui::GetIO().MouseWheel));
  }

  ImGui::End();
  ImGui::Render();
  auto draw_data = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(
      draw_data, this->m_UiCommandBuffers[frame.current_frame()]);
  frame.end_rendering(cmd);
  frame.insert_pipeline_barrier(
      cmd, this->m_Engine->renderer().swapchain()->image(frame.image_index()),
      VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
      VK_ACCESS_NONE, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
  VkPipelineStageFlags wait_stages =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  return {{}, {}, wait_stages};
}

SubmitData Editor::render_viewport(Frame &frame, Scene &scene) {
  std::uint32_t image_width =
      std::clamp<std::uint32_t>(m_ViewportSize[0], 1, 8192);
  std::uint32_t image_height =
      std::clamp<std::uint32_t>(m_ViewportSize[1], 1, 8192);

  auto cmd = m_PrimaryCommandBuffers[frame.current_frame()];
  auto image_i = frame.image_index();
  frame.begin_command_buffer(cmd);
  VkViewport viewport{};
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  viewport.width = image_width;
  viewport.height = image_height;
  viewport.x = 0;
  viewport.y = 0;

  VkRect2D scissor{};
  scissor.extent.width = image_width;
  scissor.extent.height = image_height;
  scissor.offset.x = 0;
  scissor.offset.y = 0;

  frame.set_viewport(cmd, viewport);
  frame.set_scissor(cmd, scissor);

  VulkanRenderingAttachments attachments;
  attachments.build_color_attachment()
      .clear_color(0.0f, 0.0f, 0.0f, 1.0f)
      .target(m_ViewportImageViews[image_i],
              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  attachments.build_depth_attachment().clear_depth(1.0f).target(
      m_DepthImageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

  VkRect2D render_area{};
  render_area.extent.width = image_width;
  render_area.extent.height = image_height;
  render_area.offset.x = 0;
  render_area.offset.y = 0;
  frame.insert_pipeline_barrier(
      cmd, m_ViewportImages[image_i].first, VK_IMAGE_ASPECT_COLOR_BIT,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
  frame.insert_pipeline_barrier(
      cmd, m_DepthImage.first, VK_IMAGE_ASPECT_DEPTH_BIT,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
      VK_ACCESS_NONE,
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
  frame.begin_rendering(cmd, attachments, render_area);
  frame.bind_descriptor_set(
      cmd, this->m_DescriptorSet.get(),
      this->m_Engine->pipeline_manager().get_graphics_pipeline("simple"));
  scene.record_command_buffers(cmd, *this->m_Engine, frame);
  frame.end_rendering(cmd);
  VkPipelineStageFlags wait_stages =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  clstl::vector<VkSemaphore> signal_semaphores = {
      frame.render_finished_semaphore()};
  clstl::vector<VkSemaphore> wait_semaphores = {
      frame.image_available_semaphore()};
  return {wait_semaphores, signal_semaphores, wait_stages};
}

void Editor::create_images() {
  auto &renderer = this->m_Engine->renderer();
  vkDeviceWaitIdle(renderer.device());

  this->m_ViewportImages.resize(renderer.swapchain()->image_count());
  this->m_ViewportImageViews.resize(renderer.swapchain()->image_count());
  this->m_ImGuiViewportTextures.resize(renderer.swapchain()->image_count());
  m_ViewportRenderSize = m_ViewportSize;

  std::uint32_t image_width =
      std::clamp<std::uint32_t>(m_ViewportSize[0], 1, 8192);
  std::uint32_t image_height =
      std::clamp<std::uint32_t>(m_ViewportSize[1], 1, 8192);

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

  this->m_DepthImage = renderer.create_image(
      image_width, image_height, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D32_SFLOAT,
      VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      alloc_info);
  this->m_DepthImageView =
      renderer.create_image_view(this->m_DepthImage.first, VK_FORMAT_D32_SFLOAT,
                                 VK_IMAGE_ASPECT_DEPTH_BIT);

  for (std::size_t i = 0; i < renderer.swapchain()->image_count(); i++) {
    auto viewport_image = renderer.create_image(
        image_width, image_height, VK_SAMPLE_COUNT_1_BIT,
        renderer.swapchain()->surface_format().format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        alloc_info);
    auto viewport_image_view = renderer.create_image_view(
        viewport_image.first, renderer.swapchain()->surface_format().format,
        VK_IMAGE_ASPECT_COLOR_BIT);
    m_ViewportImages[i] = viewport_image;
    m_ViewportImageViews[i] = viewport_image_view;

    m_ImGuiViewportTextures[i] = ImGui_ImplVulkan_AddTexture(
        this->m_ViewportSampler, viewport_image_view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
}

void Editor::clean_images() {
  auto &renderer = this->m_Engine->renderer();
  vkDestroyImageView(renderer.device(), this->m_DepthImageView, nullptr);
  vmaDestroyImage(renderer.allocator(), m_DepthImage.first,
                  m_DepthImage.second);
  for (std::size_t i = 0; i < m_ViewportImages.size(); i++) {
    vkDestroyImageView(renderer.device(), m_ViewportImageViews[i], nullptr);
    vmaDestroyImage(renderer.allocator(), m_ViewportImages[i].first,
                    m_ViewportImages[i].second);
    ImGui_ImplVulkan_RemoveTexture(m_ImGuiViewportTextures[i]);
  }
}

void Editor::create_descriptor_sets() {
  auto [dset, pool] = this->m_Engine->renderer()
                          .create_descriptor_sets()
                          .add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                       VK_SHADER_STAGE_VERTEX_BIT, 1, 0)
                          .build();

  this->m_DescriptorSet = dset[0];
  this->m_DescriptorPool = pool;
  this->update_camera();
}

void Editor::update_camera() {
  UniformBufferObject ubo{};
  ubo.projection = this->m_Camera->projection();
  ubo.view = this->m_Camera->view();
  auto buffer = this->m_Engine->renderer().create_uniform_buffer(ubo);

  VkDescriptorBufferInfo buffer_info{};
  buffer_info.offset = 0;
  buffer_info.buffer = buffer->handle();
  buffer_info.range = sizeof(UniformBufferObject);

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  write.pBufferInfo = &buffer_info;
  write.dstBinding = 0;
  write.dstSet = this->m_DescriptorSet->handle();

  vkUpdateDescriptorSets(this->m_Engine->renderer().device(), 1, &write, 0,
                         nullptr);
  this->m_UniformBuffer = buffer;
}

void Editor::run() {
  auto &renderer = this->m_Engine->renderer();
  m_CurrentWindowSize = this->m_Window->current_size();
  m_ViewportSize[0] = m_CurrentWindowSize.width;
  m_ViewportSize[1] = m_CurrentWindowSize.height;

  SPDLOG_INFO("Current window size: {}, {}", m_CurrentWindowSize.width,
              m_CurrentWindowSize.height);

  this->m_PrimaryCommandBuffers =
      renderer.allocate_command_buffers(renderer.swapchain()->image_count());
  this->m_UiCommandBuffers =
      renderer.allocate_command_buffers(renderer.swapchain()->image_count());

  this->m_ViewportSampler = renderer.create_sampler();
  this->create_images();

  Scene scene{};
  auto e = scene.create_entity();
  auto &registry = scene.component_registry();
  auto mesh = Mesh::load_from_file("monkey.obj", &renderer);
  MeshComponent component{mesh};
  TransformComponent transform{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(glm::pi<float>(), 0.0f, 0.0f),
                               glm::vec3(1.0f)};

  registry.add_component(e, component);
  registry.add_component(e, transform);
  this->create_descriptor_sets();
  this->build_pipelines();

  while (!this->m_Window->should_close()) {
    this->m_Input->frame_start();
    if (m_ViewportResized) {
      vkDeviceWaitIdle(renderer.device());
      this->clean_images();
      this->create_images();
    }

    auto frame = renderer.start_frame();

    if (frame.has_value()) {
      auto viewport_data = render_viewport(frame.value(), scene);
      auto ui_data = render_ui(frame.value());
      frame->add_command_buffer_for_submit(
          &m_PrimaryCommandBuffers[frame->current_frame()],
          viewport_data.signal_semaphores, viewport_data.wait_semaphores,
          &viewport_data.wait_stages);
      frame->add_command_buffer_for_submit(
          &m_UiCommandBuffers[frame->current_frame()],
          ui_data.signal_semaphores, ui_data.wait_semaphores,
          &ui_data.wait_stages);

      VkSemaphore signal_semaphores = frame->render_finished_semaphore(),
                  wait_semaphore = frame->image_available_semaphore();
      VkPipelineStageFlags wait_stage =
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

      clstl::array<VkCommandBuffer, 2> cmd_buffers = {
          this->m_PrimaryCommandBuffers[frame->current_frame()],
          m_UiCommandBuffers[frame->current_frame()]};

      VkSubmitInfo submit_info{};
      submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submit_info.signalSemaphoreCount = 1;
      submit_info.pSignalSemaphores = &signal_semaphores;
      submit_info.waitSemaphoreCount = 1;
      submit_info.pWaitSemaphores = &wait_semaphore;
      submit_info.pWaitDstStageMask = &wait_stage;
      submit_info.commandBufferCount = cmd_buffers.size();
      submit_info.pCommandBuffers = cmd_buffers.data();
      vkQueueSubmit(frame->graphics_queue(), 1, &submit_info,
                    frame->in_flight_fence());

      frame->wait_and_present();
    } else {
      vkDeviceWaitIdle(renderer.device());
      m_CurrentWindowSize = this->m_Window->current_size();
      this->clean_images();
      this->create_images();
    }
    if (m_ViewportResized) {
      this->m_CameraControls->resize(m_ViewportSize[0], m_ViewportSize[1]);
    }
    this->update_camera();
    this->m_Window->poll_events();
  }
}

Editor::~Editor() {
  this->clean_images();
  vkDestroySampler(this->m_Engine->renderer().device(), this->m_ViewportSampler,
                   nullptr);
}
} // namespace ashfault::editor
