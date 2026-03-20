#include <ashfault/core/camera.h>
#include <ashfault/core/pipeline_manager.h>
#include <ashfault/renderer/pipeline.h>
#include <ashfault/renderer/renderer.h>
#include <ashfault/renderer/swapchain.h>
#include <ashfault/renderer/vkrenderer.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

#include <ashfault/renderer/buffer.hpp>
#include <cstddef>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <limits>
#include <memory>

namespace ashfault {
struct CameraData {
  glm::mat4 projection;
  glm::mat4 view;
  glm::mat4 model;
};

struct RendererData {
  std::shared_ptr<VulkanRenderer> render_backend;
  Swapchain* swapchain;
  std::unique_ptr<PipelineManager> pipeline_manager;

  std::vector<VkCommandBuffer> command_buffers;
  std::vector<VkCommandBuffer> imgui_command_buffers;
  std::vector<VkSemaphore> image_available_semaphores;
  std::vector<VkSemaphore> render_finished_semaphores;
  std::vector<VkFence> in_flight_fences;
  VkPipelineStageFlags wait_stages;

  std::vector<VkCommandBuffer> cmd_to_submit;

  std::uint32_t image_index;
  std::uint32_t current_frame;

  std::shared_ptr<RenderTarget> default_target;
  std::vector<std::shared_ptr<RenderTarget>> targets;

  std::optional<CameraData> camera_data;
};

static RendererData s_Data;

void Renderer::create_pipelines() {
  auto static_vshader = s_Data.render_backend->create_shader("simple.vert.spv");
  auto static_fshader = s_Data.render_backend->create_shader("simple.frag.spv");

  std::vector<VkVertexInputAttributeDescription> vertex_attribs{};
  vertex_attribs.push_back({
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = 0,
  });
  vertex_attribs.push_back({
      .location = 1,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = offsetof(Mesh::Vertex, normal),
  });

  auto static_pipeline =
      s_Data.render_backend->create_graphics_pipeline()
          .input_attribute_descriptions(vertex_attribs, sizeof(Mesh::Vertex))
          .vertex_shader(static_vshader)
          .fragment_shader(static_fshader)
          .push_constant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(CameraData))
          .build();

  s_Data.pipeline_manager->add_graphics_pipeline("static", static_pipeline);
}

bool Renderer::start_frame() {
  s_Data.swapchain = s_Data.render_backend->swapchain();
  s_Data.default_target =
      create_render_target(s_Data.swapchain->swap_extent().width,
                           s_Data.swapchain->swap_extent().height, false, true);
  s_Data.cmd_to_submit.clear();
  vkWaitForFences(s_Data.render_backend->device(), 1,
                  &s_Data.in_flight_fences[s_Data.current_frame], VK_TRUE,
                  std::numeric_limits<std::uint64_t>::max());

  auto idx = s_Data.swapchain->acquire_image(
      s_Data.image_available_semaphores[s_Data.current_frame]);

  if (!idx.has_value()) {
    s_Data.render_backend->recreate_swapchain();
    return false;
  }
  vkResetFences(s_Data.render_backend->device(), 1,
                &s_Data.in_flight_fences[s_Data.current_frame]);

  s_Data.image_index = idx.value();
  s_Data.targets.clear();
  s_Data.wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkCommandBuffer cmd =
      s_Data.default_target->command_buffer(s_Data.current_frame);
  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd, &begin_info);

  s_Data.default_target->begin_rendering(s_Data.image_index,
                                         s_Data.current_frame);
  return true;
}

void Renderer::end_frame() {
  s_Data.default_target->end_rendering(s_Data.image_index, s_Data.current_frame,
                                       true);
  VkCommandBuffer cmd =
      s_Data.default_target->command_buffer(s_Data.current_frame);
  vkEndCommandBuffer(cmd);
  s_Data.cmd_to_submit.push_back(
      s_Data.default_target->command_buffer(s_Data.current_frame));
}

void Renderer::init(std::shared_ptr<Window> window) {
  s_Data.render_backend = std::make_shared<VulkanRenderer>();
  s_Data.render_backend->init(window);
  s_Data.swapchain = s_Data.render_backend->swapchain();
  s_Data.pipeline_manager = std::make_unique<PipelineManager>();
  s_Data.current_frame = 0;
  s_Data.default_target = create_render_target(
      s_Data.render_backend->swapchain()->swap_extent().width,
      s_Data.render_backend->swapchain()->swap_extent().height, false, true);
  s_Data.cmd_to_submit = {};

  s_Data.image_available_semaphores =
      s_Data.render_backend->create_semaphores(s_Data.swapchain->image_count());
  s_Data.render_finished_semaphores =
      s_Data.render_backend->create_semaphores(s_Data.swapchain->image_count());
  s_Data.in_flight_fences =
      s_Data.render_backend->create_fences(s_Data.swapchain->image_count());

  s_Data.imgui_command_buffers =
      s_Data.render_backend->allocate_command_buffers(
          s_Data.swapchain->image_count());
  s_Data.command_buffers = s_Data.render_backend->allocate_command_buffers(
      s_Data.swapchain->image_count());

  Renderer::create_pipelines();
}

void Renderer::submit_imgui_data(ImDrawData* draw_data) {
  if (draw_data) {
    ImGui_ImplVulkan_RenderDrawData(
        draw_data, s_Data.default_target->command_buffer(s_Data.current_frame));
  }
}

void Renderer::push_render_target(std::shared_ptr<RenderTarget> target) {
  s_Data.targets.push_back(target);
  VkCommandBuffer cmd = render_target().command_buffer(s_Data.current_frame);
  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd, &begin_info);

  target->begin_rendering(s_Data.image_index, s_Data.current_frame);
}

void Renderer::pop_render_target() {
  render_target().end_rendering(s_Data.image_index, s_Data.current_frame,
                                false);
  vkEndCommandBuffer(render_target().command_buffer(s_Data.current_frame));
  s_Data.cmd_to_submit.push_back(
      render_target().command_buffer(s_Data.current_frame));
  s_Data.targets.pop_back();
}

RenderTarget& Renderer::render_target() {
  return *(s_Data.targets.empty() ? s_Data.default_target
                                  : s_Data.targets.back());
}

std::shared_ptr<RenderTarget> Renderer::create_render_target(
    std::uint32_t width, std::uint32_t height, bool msaa, bool swapchain) {
  auto image_count = s_Data.swapchain->image_count();
  auto cmd_buffers =
      s_Data.render_backend->allocate_command_buffers(image_count);

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
  alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  std::vector<VkImage> images(image_count);
  std::vector<VmaAllocation> allocs(image_count);
  std::vector<VkImageView> views(image_count);

  std::pair<VkImage, VmaAllocation> depth_image =
      s_Data.render_backend->create_image(
          width, height,
          msaa ? s_Data.render_backend->msaa_samples() : VK_SAMPLE_COUNT_1_BIT,
          VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, alloc_info);
  VkImageView depth_view = s_Data.render_backend->create_image_view(
      depth_image.first, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);

  for (std::size_t i = 0; i < image_count; i++) {
    if (swapchain) {
      images[i] = s_Data.swapchain->image(i);
      views[i] = s_Data.swapchain->image_view(i);
    } else {
      auto image = s_Data.render_backend->create_image(
          width, height,
          msaa ? s_Data.render_backend->msaa_samples() : VK_SAMPLE_COUNT_1_BIT,
          s_Data.swapchain->surface_format().format, VK_IMAGE_TILING_OPTIMAL,
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
          alloc_info);
      images[i] = image.first;
      allocs[i] = image.second;

      auto view = s_Data.render_backend->create_image_view(
          image.first, s_Data.swapchain->surface_format().format,
          VK_IMAGE_ASPECT_COLOR_BIT);
      views[i] = view;
    }
  }

  VkRect2D render_area = {.offset = {.x = 0, .y = 0},
                          .extent = {.width = width, .height = height}};

  return std::make_shared<RenderTarget>(
      s_Data.render_backend, depth_image, depth_view, images, views,
      swapchain ? std::optional<std::vector<VmaAllocation>>()
                : std::make_optional(allocs),
      cmd_buffers, render_area);
}

void Renderer::submit_and_wait() {
  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = s_Data.cmd_to_submit.size();
  submit_info.pCommandBuffers = s_Data.cmd_to_submit.data();
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores =
      &s_Data.image_available_semaphores[s_Data.current_frame];
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores =
      &s_Data.render_finished_semaphores[s_Data.image_index];
  submit_info.pWaitDstStageMask = &s_Data.wait_stages;

  vkQueueSubmit(s_Data.render_backend->graphics_queue(), 1, &submit_info,
                s_Data.in_flight_fences[s_Data.current_frame]);

  std::vector<VkSemaphore> wait_semaphores{};
  wait_semaphores.push_back(
      s_Data.render_finished_semaphores[s_Data.image_index]);
  s_Data.swapchain->present(s_Data.render_backend->present_queue(),
                            wait_semaphores, s_Data.image_index);
  s_Data.current_frame = s_Data.current_frame % s_Data.swapchain->image_count();
}

void Renderer::submit_mesh(Mesh& mesh) {
  auto cmd = render_target().command_buffer(s_Data.current_frame);
  GraphicsPipeline* pipeline = nullptr;
  switch (mesh.type()) {
    case Mesh::Static:
      pipeline = s_Data.pipeline_manager->get_graphics_pipeline("static");

      break;
  }

  auto data = s_Data.camera_data.value_or<CameraData>(
      {.projection = glm::identity<glm::mat4>(),
       .view = glm::identity<glm::mat4>(),
       .model = glm::identity<glm::mat4>()});
  data.model = glm::identity<glm::mat4>();
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle());
  vkCmdPushConstants(cmd, pipeline->layout(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                     sizeof(CameraData), &data);
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertex_buffer()->handle(), &offset);
  vkCmdDraw(cmd, mesh.vertex_buffer()->count(), 1, 0, 0);
}

void Renderer::shutdown() {
  vkDeviceWaitIdle(s_Data.render_backend->device());
  s_Data.default_target.reset();
  s_Data.targets.clear();
  s_Data.render_backend->shutdown();
}

VulkanRenderer& Renderer::vulkan_backend() { return *s_Data.render_backend; }

std::uint32_t Renderer::swapchain_image_index() { return s_Data.image_index; }

std::shared_ptr<Mesh> Renderer::create_mesh(
    Mesh::MeshType type, const std::vector<Mesh::Vertex>& vertices,
    const std::vector<std::uint32_t>& indices) {
  auto vertex_buffer = s_Data.render_backend->create_vertex_buffer(vertices);
  auto index_buffer = s_Data.render_backend->create_index_buffer(indices);
  return std::make_shared<Mesh>(type, vertex_buffer, index_buffer);
}

void Renderer::begin_scene(Camera& camera) {
  CameraData data = {.projection = camera.projection(), .view = camera.view()};

  s_Data.camera_data = data;
}

void Renderer::end_scene() { s_Data.camera_data = std::optional<CameraData>(); }

std::uint32_t Renderer::frame_index() { return s_Data.current_frame; }
}  // namespace ashfault
