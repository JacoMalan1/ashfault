#include <ashfault/renderer/descriptor_set.h>
#include <ashfault/renderer/pipeline.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <algorithm>

namespace ashfault {
GraphicsPipeline::GraphicsPipeline(VkDevice device, VkPipelineLayout layout,
                                   VkPipeline pipeline)
    : m_Layout(layout), m_Pipeline(pipeline), m_Device(device) {}

GraphicsPipeline::~GraphicsPipeline() {
  vkDeviceWaitIdle(this->m_Device);
  vkDestroyPipeline(this->m_Device, this->m_Pipeline, nullptr);
  vkDestroyPipelineLayout(this->m_Device, this->m_Layout, nullptr);
}

GraphicsPipelineBuilder::GraphicsPipelineBuilder(
    VkDevice device, VkFormat swapchain_image_format,
    std::array<std::uint32_t, 2> window_dims,
    VkSampleCountFlagBits msaa_samples)
    : m_VertexShader(), m_FragmentShader(), m_DescriptorSets(),
      m_Device(device), m_ImageFormat(swapchain_image_format),
      m_MsaaSamples(msaa_samples), m_WindowDims(std::move(window_dims)) {}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::push_constant(VkShaderStageFlags stage,
                                       VkDeviceSize offset, VkDeviceSize size) {
  VkPushConstantRange range{};
  range.stageFlags = stage;
  range.offset = offset;
  range.size = size;

  this->m_PushConstants.push_back(range);
  return *this;
}

std::shared_ptr<GraphicsPipeline> GraphicsPipelineBuilder::build() {
  std::vector<VkDescriptorSetLayout> dset_layouts;
  dset_layouts.reserve(this->m_DescriptorSets.size());
  std::for_each(m_DescriptorSets.begin(), m_DescriptorSets.end(),
                  [&](std::shared_ptr<VulkanDescriptorSet> set) {
                    dset_layouts.push_back(set->layout());
                  });

  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = dset_layouts.size();
  pipeline_layout_info.pSetLayouts =
      dset_layouts.empty() ? nullptr : dset_layouts.data();
  pipeline_layout_info.pushConstantRangeCount = m_PushConstants.size();
  pipeline_layout_info.pPushConstantRanges = m_PushConstants.data();

  VkPipelineLayout pipeline_layout;
  if (vkCreatePipelineLayout(this->m_Device, &pipeline_layout_info, nullptr,
                             &pipeline_layout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout");
  }

  VkPipelineShaderStageCreateInfo vshader_stage_info{};
  vshader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vshader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vshader_stage_info.module = this->m_VertexShader.value()->handle();
  vshader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo fshader_stage_info{};
  fshader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fshader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fshader_stage_info.module = this->m_FragmentShader.value()->handle();
  fshader_stage_info.pName = "main";

  std::array<VkPipelineShaderStageCreateInfo, 2> stages = {
      vshader_stage_info, fshader_stage_info};

  std::array<VkDynamicState, 2> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                                    VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamic_state_info{};
  dynamic_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_info.dynamicStateCount = dynamic_states.size();
  dynamic_state_info.pDynamicStates = dynamic_states.data();

  VkPipelineColorBlendAttachmentState blend_attachment{};
  blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
  blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

  VkPipelineColorBlendStateCreateInfo blend_info{};
  blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend_info.logicOpEnable = VK_FALSE;
  blend_info.attachmentCount = 1;
  blend_info.pAttachments = &blend_attachment;

  VkPipelineMultisampleStateCreateInfo multisample_info{};
  multisample_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample_info.rasterizationSamples = this->m_MsaaSamples;
  multisample_info.sampleShadingEnable = VK_FALSE;

  VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};
  depth_stencil_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
  depth_stencil_info.depthTestEnable = VK_TRUE;
  depth_stencil_info.depthWriteEnable = VK_TRUE;
  depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
  depth_stencil_info.stencilTestEnable = VK_FALSE;

  VkPipelineRenderingCreateInfo rendering_info{};
  rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  rendering_info.colorAttachmentCount = 1;
  rendering_info.pColorAttachmentFormats = &this->m_ImageFormat;
  rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

  VkViewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = this->m_WindowDims[0];
  viewport.height = this->m_WindowDims[1];
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;
  viewport_state.pViewports = &viewport;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.depthClampEnable = VK_FALSE;

  VkPipelineInputAssemblyStateCreateInfo default_assembly_state{};
  default_assembly_state.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  default_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  default_assembly_state.primitiveRestartEnable = VK_FALSE;
  auto assembly_state =
      this->m_AssemblyState.value_or(std::move(default_assembly_state));

  VkVertexInputBindingDescription bindings{};
  bindings.binding = 0;
  bindings.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  bindings.stride = this->m_VertexStride;

  VkPipelineVertexInputStateCreateInfo vertex_input{};
  vertex_input.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input.vertexBindingDescriptionCount = 1;
  vertex_input.pVertexBindingDescriptions = &bindings;
  vertex_input.pVertexAttributeDescriptions = this->m_VertexAttributes.data();
  vertex_input.vertexAttributeDescriptionCount =
      this->m_VertexAttributes.size();

  VkGraphicsPipelineCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  create_info.pNext = &rendering_info;
  create_info.stageCount = stages.size();
  create_info.pStages = stages.data();
  create_info.pDynamicState = &dynamic_state_info;
  create_info.layout = pipeline_layout;
  create_info.pColorBlendState = &blend_info;
  create_info.pMultisampleState = &multisample_info;
  create_info.pDepthStencilState = &depth_stencil_info;
  create_info.pViewportState = &viewport_state;
  create_info.pRasterizationState = &rasterizer;
  create_info.pInputAssemblyState = &assembly_state;
  create_info.pVertexInputState = &vertex_input;

  VkPipeline pipeline;
  if (vkCreateGraphicsPipelines(this->m_Device, VK_NULL_HANDLE, 1, &create_info,
                                nullptr, &pipeline) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create graphics pipeline");
  }

  return std::make_shared<GraphicsPipeline>(this->m_Device, pipeline_layout,
                                              pipeline);
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::input_attribute_descriptions(
    const std::vector<VkVertexInputAttributeDescription> &descriptions,
    std::uint32_t stride) {
  this->m_VertexAttributes.reserve(descriptions.size());
  std::for_each(descriptions.begin(), descriptions.end(),
                  [&](VkVertexInputAttributeDescription desc) {
                    this->m_VertexAttributes.push_back(desc);
                  });
  this->m_VertexStride = stride;
  return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::input_assembly_state(
    VkPipelineInputAssemblyStateCreateInfo assembly_state) {
  this->m_AssemblyState = assembly_state;
  return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::vertex_shader(std::shared_ptr<VulkanShader> shader) {
  this->m_VertexShader = shader;
  return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::fragment_shader(
    std::shared_ptr<VulkanShader> shader) {
  this->m_FragmentShader = shader;
  return *this;
}

VkPipeline GraphicsPipeline::handle() const { return this->m_Pipeline; }

GraphicsPipelineBuilder &GraphicsPipelineBuilder::descriptor_sets(
    const std::vector<std::shared_ptr<VulkanDescriptorSet>> &dsets) {
  std::for_each(dsets.begin(), dsets.end(),
                  [&](std::shared_ptr<VulkanDescriptorSet> set) {
                    this->m_DescriptorSets.push_back(set);
                  });
  return *this;
}

const VkPipelineLayout &GraphicsPipeline::layout() const {
  return this->m_Layout;
}

VkPipelineLayout &GraphicsPipeline::layout() { return this->m_Layout; }
} // namespace ashfault
