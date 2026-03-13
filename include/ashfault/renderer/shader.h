#ifndef ASHFAULT_SHADER_H
#define ASHFAULT_SHADER_H

#include <vulkan/vulkan_core.h>
#include <ashfault/ashfault.h>
#include <string>

namespace ashfault {
class ASHFAULT_API VulkanShader {
public:
  VulkanShader(VkDevice device, const std::string &path);
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
