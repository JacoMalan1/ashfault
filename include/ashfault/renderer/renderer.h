#ifndef ASHFAULT_RENDERER_H
#define ASHFAULT_RENDERER_H

#include <ashfault/ashfault.h>
#include <ashfault/core/camera.h>
#include <ashfault/core/mesh.h>
#include <ashfault/core/window.h>
#include <ashfault/renderer/target.h>
#include <imgui.h>

#include <memory>

namespace ashfault {
class ASHFAULT_API Renderer {
 public:
  static void init(std::shared_ptr<Window> window);
  static void shutdown();
  static bool start_frame();
  static void end_frame();

  static void push_render_target(std::shared_ptr<RenderTarget> target);
  static void pop_render_target();

  static RenderTarget &render_target();

  static void begin_scene(Camera &camera);
  static void end_scene();

  static std::shared_ptr<RenderTarget> create_render_target(
      std::uint32_t width, std::uint32_t height, bool msaa = true,
      bool swapchain = false);

  static void submit_mesh(Mesh &mesh, const glm::mat4 &transform);
  static void submit_mesh(Mesh &mesh);

  static std::shared_ptr<Mesh> create_mesh(
      Mesh::MeshType type, const std::vector<Mesh::Vertex> &vertices,
      const std::vector<std::uint32_t> &indices);

  static void submit_imgui_data(ImDrawData *);

  static void submit_and_wait();

  static VulkanRenderer &vulkan_backend();
  static std::uint32_t swapchain_image_index();
  static std::uint32_t frame_index();

 private:
  static void create_pipelines();
};
}  // namespace ashfault

#endif
