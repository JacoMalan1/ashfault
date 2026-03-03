#ifndef ASHFAULT_PIPELINE_H
#define ASHFAULT_PIPELINE_H

#include <CLSTL/array.h>
#include <CLSTL/shared_ptr.h>
#include <CLSTL/vector.h>
#include <ashfault/descriptor_set.h>
#include <ashfault/shader.h>
#include <optional>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace ashfault {
class GraphicsPipeline;

class GraphicsPipelineBuilder {
public:
  GraphicsPipelineBuilder(VkDevice device, VkFormat swapchain_image_format,
                          clstl::array<std::uint32_t, 2> window_dims);
  ~GraphicsPipelineBuilder();

  GraphicsPipelineBuilder &
  vertex_shader(clstl::shared_ptr<VulkanShader> shader);
  GraphicsPipelineBuilder &
  fragment_shader(clstl::shared_ptr<VulkanShader> shader);
  GraphicsPipelineBuilder &descriptor_sets(
      const clstl::vector<clstl::shared_ptr<VulkanDescriptorSet>> &dsets);
  GraphicsPipelineBuilder &input_attribute_descriptions(
      const clstl::vector<VkVertexInputAttributeDescription> &descriptions, std::uint32_t stride);
  GraphicsPipelineBuilder &
  input_assembly_state(VkPipelineInputAssemblyStateCreateInfo assembly_state);

  clstl::shared_ptr<GraphicsPipeline> build();

private:
  std::optional<clstl::shared_ptr<VulkanShader>> m_VertexShader;
  std::optional<clstl::shared_ptr<VulkanShader>> m_FragmentShader;
  std::optional<VkPipelineInputAssemblyStateCreateInfo> m_AssemblyState;
  clstl::vector<clstl::shared_ptr<VulkanDescriptorSet>> m_DescriptorSets;
  clstl::vector<VkVertexInputAttributeDescription> m_VertexAttributes;
  std::uint32_t m_VertexStride;
  VkDevice m_Device;
  VkFormat m_ImageFormat;
  clstl::array<std::uint32_t, 2> m_WindowDims;
};

class GraphicsPipeline {
public:
  GraphicsPipeline(VkDevice device, VkPipelineLayout layout,
                   VkPipeline pipeline);

  GraphicsPipeline(const GraphicsPipeline &) = delete;
  GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
  ~GraphicsPipeline();

  VkPipeline handle() const;
  const VkPipelineLayout &layout() const;
  VkPipelineLayout &layout();

private:
  VkPipelineLayout m_Layout;
  VkPipeline m_Pipeline;
  VkDevice m_Device;
};
} // namespace ashfault

#endif
