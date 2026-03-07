#include <CLSTL/shared_ptr.h>
#include <ashfault/core/af_window.h>
#include <ashfault/core/component/mesh.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/scene.h>
#include <ashfault/editor/editor.h>
#include <ashfault/renderer/frame.h>
#include <ashfault/renderer/renderer.h>
#include <ashfault/renderer/swapchain.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

namespace ashfault {

Editor::Editor(clstl::shared_ptr<Engine> engine,
               clstl::shared_ptr<Window> window)
    : Application(engine, window),
      m_ViewportSize({(std::uint32_t)1, (std::uint32_t)1}),
      m_CurrentWindowSize({1, 1}), m_ViewportResized(false) {}

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
  ImGui::DockSpaceOverViewport();
  ImGui::Begin("Viewport");
  auto size = ImGui::GetContentRegionAvail();
  if (size.x != m_ViewportSize[0] || size.y != m_ViewportSize[1]) {
    m_ViewportResized = true;
  }
  m_ViewportSize[0] = size.x;
  m_ViewportSize[1] = size.y;
  ImGui::Image(this->m_ImGuiViewportTextures[frame.image_index()], size);
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
  auto cmd = m_PrimaryCommandBuffers[frame.current_frame()];
  auto image_i = frame.image_index();
  frame.begin_command_buffer(cmd);
  VkViewport viewport{};
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  viewport.width = m_ViewportRenderSize[0];
  viewport.height = m_ViewportRenderSize[1];
  viewport.x = 0;
  viewport.y = 0;

  VkRect2D scissor{};
  scissor.extent.width = m_ViewportRenderSize[0];
  scissor.extent.height = m_ViewportRenderSize[1];
  scissor.offset.x = 0;
  scissor.offset.y = 0;

  frame.set_viewport(cmd, viewport);
  frame.set_scissor(cmd, scissor);

  VulkanRenderingAttachments attachments;
  attachments.build_color_attachment()
      .clear_color(0.0f, 0.0f, 0.0f, 1.0f)
      .target(m_ViewportImageViews[image_i],
              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  VkRect2D render_area{};
  render_area.extent.width = m_ViewportRenderSize[0];
  render_area.extent.height = m_ViewportRenderSize[1];
  render_area.offset.x = 0;
  render_area.offset.y = 0;
  frame.insert_pipeline_barrier(
      cmd, m_ViewportImages[image_i].first, VK_IMAGE_ASPECT_COLOR_BIT,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
  frame.begin_rendering(cmd, attachments, render_area);
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

  for (std::size_t i = 0; i < renderer.swapchain()->image_count(); i++) {
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    auto viewport_image = renderer.create_image(
        m_ViewportSize[0], m_ViewportSize[1], VK_SAMPLE_COUNT_1_BIT,
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
  for (std::size_t i = 0; i < m_ViewportImages.size(); i++) {
    vkDestroyImageView(renderer.device(), m_ViewportImageViews[i], nullptr);
    vmaDestroyImage(renderer.allocator(), m_ViewportImages[i].first,
                    m_ViewportImages[i].second);
    ImGui_ImplVulkan_RemoveTexture(m_ImGuiViewportTextures[i]);
  }
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
  registry.add_component<TransformComponent>(
      e, {glm::vec3(0), glm::vec3(0), glm::vec3(1)});

  clstl::vector<Vertex> vertices;
  vertices.push_back({glm::vec3(0.0f, -0.5f, 0.0f)});
  vertices.push_back({glm::vec3(-0.5f, 0.5f, 0.0f)});
  vertices.push_back({glm::vec3(0.5f, 0.5f, 0.0f)});

  clstl::vector<std::uint16_t> indices;
  indices.push_back(0);
  indices.push_back(1);
  indices.push_back(2);

  auto vbuf = renderer.create_vertex_buffer(vertices);
  auto ibuf = renderer.create_index_buffer(indices);

  MeshComponent<Vertex, std::uint16_t> mesh;
  mesh.vertex_buffer = vbuf;
  mesh.index_buffer = ibuf;

  registry.add_component<MeshComponent<Vertex, std::uint16_t>>(e, mesh);

  while (!this->m_Window->should_close()) {
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
    this->m_Window->poll_events();
  }
}

Editor::~Editor() {
  this->clean_images();
  vkDestroySampler(this->m_Engine->renderer().device(), this->m_ViewportSampler,
                   nullptr);
}
} // namespace ashfault
