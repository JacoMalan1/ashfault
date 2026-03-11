#include <algorithm>
#include <ashfault/core/engine.h>
#include <ashfault/core/pipeline_manager.h>
#include <ashfault/core/scene.h>
#include <ashfault/renderer/pipeline.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

namespace ashfault {
Engine::Engine()
    : m_Renderer(std::make_unique<Renderer>()),
      m_ShaderManager(std::make_unique<ShaderManager>()),
      m_PipelineManager(std::make_unique<PipelineManager>()) {}

Engine::~Engine() {}

Renderer &Engine::renderer() { return *this->m_Renderer; }

const Renderer &Engine::renderer() const { return *this->m_Renderer; }

void Engine::register_shaders() {
  SPDLOG_INFO("Registering shaders");

  std::vector<std::string> shaders;
  shaders.emplace_back("blinn_phong");
  shaders.emplace_back("simple");

  std::for_each(shaders.begin(), shaders.end(), [&](std::string name) {
    SPDLOG_INFO("Registering shader: \"{}\"", name);
    this->m_ShaderManager->add_vertex_shader(this->m_Renderer.get(),
                                             name + ".vert.spv", name);
    this->m_ShaderManager->add_fragment_shader(this->m_Renderer.get(),
                                               name + ".frag.spv", name);
  });
}

void Engine::create_pipelines() {
}

void Engine::setup_renderer(std::shared_ptr<Window> window) {
  SPDLOG_INFO("Starting Vulkan renderer");
  this->m_Renderer->init(window);
  this->register_shaders();
  this->create_pipelines();
}

PipelineManager &Engine::pipeline_manager() { return *this->m_PipelineManager; }
ShaderManager &Engine::shader_manager() { return *this->m_ShaderManager; }
} // namespace ashfault
