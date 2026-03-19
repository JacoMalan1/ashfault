#ifndef ASHFAULT_RENDERER_H
#define ASHFAULT_RENDERER_H

#include <ashfault/core/mesh.h>
#include <imgui.h>
#include <ashfault/ashfault.h>
#include <memory>
#include <ashfault/core/window.h>
#include <ashfault/renderer/target.h>

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

  static void begin_scene();
  static void end_scene();

  static std::shared_ptr<RenderTarget> create_render_target(bool msaa = true, bool swapchain = false);

  static void submit_mesh(Mesh &mesh);

  static void submit_imgui_data(ImDrawData *);

  static void submit_and_wait();
};
}

#endif
