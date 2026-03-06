#include <ashfault/core/shader_manager.h>

namespace ashfault {
ShaderManager::ShaderManager() {}

void ShaderManager::add_vertex_shader(Renderer *renderer, const std::string &path, const std::string &name) {
  auto shader = renderer->create_shader(path.c_str());
  this->m_VertexShaders[name] = shader;
}

void ShaderManager::add_fragment_shader(Renderer *renderer, const std::string &path, const std::string &name) {
  auto shader = renderer->create_shader(path.c_str());
  this->m_FragmentShaders[name] = shader;
}


std::optional<clstl::shared_ptr<VulkanShader>> ShaderManager::get_vertex_shader(const std::string &name) {
  if (!this->m_VertexShaders.count(name))
    return {};

  return this->m_VertexShaders[name];
}

std::optional<clstl::shared_ptr<VulkanShader>> ShaderManager::get_fragment_shader(const std::string &name) {
  if (!this->m_FragmentShaders.count(name))
    return {};

  return this->m_FragmentShaders[name];
}
}
