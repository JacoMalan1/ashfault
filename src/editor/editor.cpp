#include "ashfault/core/component/mesh.h"
#include <CLSTL/shared_ptr.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/scene.h>
#include <ashfault/editor/editor.h>
#include <ashfault/renderer/frame.h>
#include <ashfault/renderer/swapchain.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

namespace ashfault {

Editor::Editor(clstl::shared_ptr<Engine> engine,
               clstl::shared_ptr<Window> window)
    : Application(engine, window) {}

void Editor::run() {
  auto &renderer = this->m_Engine->renderer();
  clstl::vector<VkCommandBuffer> primary_cmd_buffers =
      renderer.allocate_command_buffers(renderer.swapchain()->image_count());
  clstl::vector<VkCommandBuffer> ui_cmd_buffers =
      renderer.allocate_command_buffers(renderer.swapchain()->image_count());

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
    auto frame = renderer.start_frame();

    if (frame.has_value()) {
      auto cmd = primary_cmd_buffers[frame->image_index()];
      auto image_i = frame->image_index();
      frame->begin_command_buffer(cmd);
      VulkanRenderingAttachments attachments;
      attachments.build_color_attachment()
          .clear_color(0.0f, 0.0f, 0.0f, 1.0f)
          .target(renderer.swapchain()->image_view(image_i),
                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

      WindowDims dims = this->m_Window->current_size();
      VkRect2D render_area{};
      render_area.extent.width = dims.width;
      render_area.extent.height = dims.height;
      render_area.offset.x = 0;
      render_area.offset.y = 0;
      frame->begin_rendering(cmd, attachments, render_area);
      scene.record_command_buffers(cmd, *this->m_Engine, frame.value());
      frame->end_rendering(cmd);
      VkPipelineStageFlags wait_stages =
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      frame->add_command_buffer_for_submit(&cmd, {}, {}, &wait_stages);
      frame->submit_all(VK_NULL_HANDLE);
      frame->wait_and_present();
    }

    this->m_Window->poll_events();
  }
}
} // namespace ashfault
