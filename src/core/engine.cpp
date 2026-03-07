#include <CLSTL/algorithm.h>
#include <CLSTL/unique_ptr.h>
#include <algorithm>
#include <ashfault/core/engine.h>
#include <ashfault/core/pipeline_manager.h>
#include <ashfault/core/scene.h>
#include <ashfault/renderer/pipeline.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

namespace ashfault {
Engine::Engine()
    : m_Renderer(clstl::make_unique<Renderer>()),
      m_ShaderManager(clstl::make_unique<ShaderManager>()),
      m_PipelineManager(clstl::make_unique<PipelineManager>()) {}

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
  clstl::vector<VkVertexInputAttributeDescription> descriptions;
  descriptions.resize(1);
  descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  descriptions[0].offset = 0;
  descriptions[0].binding = 0;
  descriptions[0].location = 0;

  auto simple =
      m_Renderer->create_graphics_pipeline()
          .input_attribute_descriptions(descriptions, sizeof(Vertex))
          .vertex_shader(m_ShaderManager->get_vertex_shader("simple").value())
          .fragment_shader(
              m_ShaderManager->get_fragment_shader("simple").value())
          .build();

  this->m_PipelineManager->add_graphics_pipeline("simple", simple);
}

void Engine::setup_renderer(clstl::shared_ptr<Window> window) {
  SPDLOG_INFO("Starting Vulkan renderer");
  this->m_Renderer->init(window);
  this->register_shaders();
  this->create_pipelines();
}

PipelineManager &Engine::pipeline_manager() { return *this->m_PipelineManager; }
} // namespace ashfault
