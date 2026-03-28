#ifndef ASHFAULT_RENDERER_H
#define ASHFAULT_RENDERER_H

#include <ashfault/ashfault.h>
#include <ashfault/core/camera.h>
#include <ashfault/core/material.h>
#include <ashfault/core/mesh.h>
#include <ashfault/core/window.h>
#include <ashfault/renderer/light.h>
#include <ashfault/renderer/target.h>
#include <imgui.h>

#include <memory>

#define ASHFAULT_MAX_LIGHTS 128
#define ASHFAULT_MAX_TEXTURES 1000

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

  static void submit_mesh(Mesh &mesh, const glm::mat4 &transform,
                          const Material &material);
  static void submit_mesh(Mesh &mesh);
  static void add_light(const Light &light);

  static std::shared_ptr<Mesh> create_mesh(
      Mesh::MeshType type, const std::vector<Mesh::Vertex> &vertices,
      const std::vector<std::uint32_t> &indices);

  static void submit_imgui_data(ImDrawData *);

  static void submit_and_wait();

  static VulkanRenderer &vulkan_backend();
  static std::uint32_t swapchain_image_index();
  static std::uint32_t frame_index();

  static int upload_texture(const char *pixels, std::uint32_t width,
                                      std::uint32_t height);

private:
  static void create_pipelines();
  static void create_descriptors();
};
}  // namespace ashfault

#endif
