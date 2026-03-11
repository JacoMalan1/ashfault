#include <ashfault/renderer/shader.h>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <vector>

namespace ashfault {
VulkanShader::VulkanShader(VkDevice device, const std::string &path)
    : m_Module(VK_NULL_HANDLE), m_Device(device) {
  std::ifstream fs(path.c_str(), std::ios_base::binary | std::ios_base::in);

  if (!fs.is_open()) {
    throw std::runtime_error("Failed to read shader module");
  }

  fs.seekg(0, std::ios::end);
  std::size_t file_size = fs.tellg();

  if (file_size % sizeof(std::uint32_t) != 0) {
    throw std::runtime_error("Invalid shader module file");
  }

  fs.seekg(0, std::ios::beg);
  std::vector<char> buf;
  buf.resize(file_size);
  fs.read(buf.data(), file_size);
  std::vector<std::uint32_t> data;
  data.resize(file_size / sizeof(std::uint32_t));
  std::memcpy(data.data(), buf.data(), file_size);

  VkShaderModuleCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.codeSize = file_size;
  info.pCode = data.data();

  if (vkCreateShaderModule(device, &info, nullptr, &this->m_Module) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module");
  }
}

VulkanShader::~VulkanShader() {
  vkDestroyShaderModule(this->m_Device, this->m_Module, nullptr);
}

VkShaderModule VulkanShader::handle() const { return this->m_Module; }
} // namespace ashfault
