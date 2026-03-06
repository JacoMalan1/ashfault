#ifndef ASHFAULT_ENGINE_H
#define ASHFAULT_ENGINE_H

#include <ashfault/core/shader_manager.h>
#include <ashfault/core/pipeline_manager.h>
#include <CLSTL/unique_ptr.h>
#include <ashfault/renderer/renderer.h>

namespace ashfault {
class Engine {
public:
  Engine();
  Engine(const Engine &) = delete;
  Engine &operator=(const Engine &) = delete;
  ~Engine();

  void setup_renderer(clstl::shared_ptr<Window> window);
  PipelineManager &pipeline_manager();

  Renderer &renderer();
  const Renderer &renderer() const;

private:
  void register_shaders();
  void create_pipelines();

  clstl::unique_ptr<Renderer> m_Renderer;
  clstl::unique_ptr<ShaderManager> m_ShaderManager;
  clstl::unique_ptr<PipelineManager> m_PipelineManager;
};
}

#endif
