#ifndef ASHFAULT_DESCRIPTOR_SET_H
#define ASHFAULT_DESCRIPTOR_SET_H

#include "CLSTL/vector.h"
#include <ashfault/renderer/buffer.hpp>
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

  template<class T>
  void update_uniform_buffer(clstl::shared_ptr<VulkanBuffer<T>> buffer, std::uint32_t binding) {
    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = buffer->handle();
    buffer_info.offset = 0;
    buffer_info.range = static_cast<VkDeviceSize>(sizeof(T));

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pBufferInfo = &buffer_info;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.dstBinding = binding;
    write.dstSet = this->m_DescriptorSet;

    vkDeviceWaitIdle(this->m_Device);
    vkUpdateDescriptorSets(this->m_Device, 1, &write, 0, nullptr);
  }

private:
  VmaAllocator m_Allocator;
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
