#ifndef ASHFAULT_ENGINE_H
#define ASHFAULT_ENGINE_H

#include <memory>
#include <ashfault/core/pipeline_manager.h>
#include <ashfault/core/shader_manager.h>
#include <ashfault/renderer/vkrenderer.h>
#include <ashfault/ashfault.h>

namespace ashfault {
class ASHFAULT_API Engine {
public:
  Engine();
  Engine(const Engine &) = delete;
  Engine &operator=(const Engine &) = delete;
  ~Engine();

  void setup_renderer(std::shared_ptr<Window> window);
  PipelineManager &pipeline_manager();
  ShaderManager &shader_manager();

  VulkanRenderer &renderer();
  const VulkanRenderer &renderer() const;

private:
  void register_shaders();
  void create_pipelines();

  std::unique_ptr<VulkanRenderer> m_Renderer;
  std::unique_ptr<ShaderManager> m_ShaderManager;
  std::unique_ptr<PipelineManager> m_PipelineManager;
};
} // namespace ashfault

#endif
