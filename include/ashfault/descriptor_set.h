#ifndef ASHFAULT_DESCRIPTOR_SET_H
#define ASHFAULT_DESCRIPTOR_SET_H

#include "CLSTL/vector.h"
#include <CLSTL/shared_ptr.h>
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace ashfault {
class VulkanDescriptorSet;
class VulkanDescriptorPool;

class VulkanDescriptorSetBuilder {
public:
  explicit VulkanDescriptorSetBuilder(VkDevice device);

  /// @brief Adds a binding to the descriptor set being built.
  ///
  /// @param type The type of binding to add.
  /// @param stage_flags The shader stages that will use this binding.
  /// @param descriptor_count How many descriptors to create.
  /// @param binding The binding index.
  VulkanDescriptorSetBuilder &add_binding(VkDescriptorType type, VkShaderStageFlags stage_flags,
                   std::uint32_t descriptor_count, std::uint32_t binding);

  /// @brief Builds the descriptor set and creates a descriptor pool.
  std::pair<clstl::vector<clstl::shared_ptr<VulkanDescriptorSet>>,
            clstl::shared_ptr<VulkanDescriptorPool>>
  build();

private:
  clstl::vector<VkDescriptorSetLayoutBinding> m_Bindings;
  VkDevice m_Device;
};

class VulkanDescriptorSet {
public:
  VulkanDescriptorSet(VkDevice device, VkDescriptorSet descriptor_set,
                      VkDescriptorSetLayout layout);

  VkDescriptorSetLayout &layout();
  const VkDescriptorSetLayout &layout() const;

  const VkDescriptorSet &handle() const;
  VkDescriptorSet &handle();

  VulkanDescriptorSet(const VulkanDescriptorSet &) = delete;
  VulkanDescriptorSet &operator=(const VulkanDescriptorSet &) = delete;
  ~VulkanDescriptorSet();

private:
  VkDevice m_Device;
  VkDescriptorSet m_DescriptorSet;
  VkDescriptorSetLayout m_Layout;
};

class VulkanDescriptorPool {
public:
  VulkanDescriptorPool(VkDevice device, VkDescriptorPool pool);

  VulkanDescriptorPool(const VulkanDescriptorPool &) = delete;
  VulkanDescriptorPool &operator=(const VulkanDescriptorPool &) = delete;
  ~VulkanDescriptorPool();

private:
  VkDescriptorPool m_Pool;
  VkDevice m_Device;
};
} // namespace ashfault

#endif
