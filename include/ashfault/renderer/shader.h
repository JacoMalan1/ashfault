#ifndef ASHFAULT_SHADER_H
#define ASHFAULT_SHADER_H

#include <CLSTL/string.h>
#include <vulkan/vulkan_core.h>

namespace ashfault {
class VulkanShader {
public:
  VulkanShader(VkDevice device, const clstl::string &path);
  VulkanShader(const VulkanShader &) = delete;
  VulkanShader &operator=(const VulkanShader &) = delete;
  ~VulkanShader();

  VkShaderModule handle() const;

private:
  VkShaderModule m_Module;
  VkDevice m_Device;
};
} // namespace ashfault

#endif
