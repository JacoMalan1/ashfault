#ifndef ASHFAULT_SHADER_MANAGER_H
#define ASHFAULT_SHADER_MANAGER_H

#include <ashfault/renderer/renderer.h>
#include <ashfault/ashfault.h>
#include <ashfault/renderer/shader.h>
#include <string>
#include <unordered_map>

namespace ashfault {
class ASHFAULT_API ShaderManager {
public:
  ShaderManager();

  void add_vertex_shader(Renderer *renderer, const std::string &path,
                         const std::string &name);
  void add_fragment_shader(Renderer *renderer, const std::string &path,
                           const std::string &name);

  std::optional<std::shared_ptr<VulkanShader>>
  get_vertex_shader(const std::string &name);
  std::optional<std::shared_ptr<VulkanShader>>
  get_fragment_shader(const std::string &name);

private:
  std::unordered_map<std::string, std::shared_ptr<VulkanShader>>
      m_VertexShaders;
  std::unordered_map<std::string, std::shared_ptr<VulkanShader>>
      m_FragmentShaders;
};
} // namespace ashfault

#endif
