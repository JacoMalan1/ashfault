#ifndef ASHFAULT_PIPELINE_H
#define ASHFAULT_PIPELINE_H

#include <ashfault/ashfault.h>
#include <ashfault/renderer/descriptor_set.h>
#include <ashfault/renderer/shader.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <array>
#include <optional>

namespace ashfault {
class ASHFAULT_API GraphicsPipeline;

class ASHFAULT_API GraphicsPipelineBuilder {
public:
  GraphicsPipelineBuilder(VkDevice device, VkFormat swapchain_image_format,
                          std::array<std::uint32_t, 2> window_dims,
                          VkSampleCountFlagBits msaaSamples);

  GraphicsPipelineBuilder &vertex_shader(std::shared_ptr<VulkanShader> shader);
  GraphicsPipelineBuilder &fragment_shader(
      std::shared_ptr<VulkanShader> shader);
  GraphicsPipelineBuilder &input_attribute_descriptions(
      const std::vector<VkVertexInputAttributeDescription> &descriptions,
      std::uint32_t stride);
  GraphicsPipelineBuilder &input_assembly_state(
      VkPipelineInputAssemblyStateCreateInfo assembly_state);
  GraphicsPipelineBuilder &push_constant(VkShaderStageFlags stage,
                                         VkDeviceSize offset,
                                         VkDeviceSize size);

  std::shared_ptr<GraphicsPipeline> build(
      const std::vector<VkDescriptorSetLayout> &dset_layouts);

private:
  std::optional<std::shared_ptr<VulkanShader>> m_VertexShader;
  std::optional<std::shared_ptr<VulkanShader>> m_FragmentShader;
  std::optional<VkPipelineInputAssemblyStateCreateInfo> m_AssemblyState;
  std::vector<VkVertexInputAttributeDescription> m_VertexAttributes;
  std::vector<VkPushConstantRange> m_PushConstants;
  std::uint32_t m_VertexStride;
  VkDevice m_Device;
  VkFormat m_ImageFormat;
  VkSampleCountFlagBits m_MsaaSamples;
  std::array<std::uint32_t, 2> m_WindowDims;
};

class ASHFAULT_API GraphicsPipeline {
public:
  GraphicsPipeline(VkDevice device, VkPipelineLayout layout,
                   VkPipeline pipeline);

  GraphicsPipeline(const GraphicsPipeline &) = delete;
  GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
  void destroy();

  VkPipeline handle() const;
  const VkPipelineLayout &layout() const;
  VkPipelineLayout &layout();

private:
  VkPipelineLayout m_Layout;
  VkPipeline m_Pipeline;
  VkDevice m_Device;
};
}  // namespace ashfault

#endif
