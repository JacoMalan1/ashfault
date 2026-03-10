#ifndef ASHFAULT_BUFFER_H
#define ASHFAULT_BUFFER_H

#include <cstdint>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace ashfault {
/// @private
template <class I> class index_type;

/// @private
template <> class index_type<std::uint32_t> {
public:
  static const VkIndexType value = VK_INDEX_TYPE_UINT32;
};

/// @private
template <> class index_type<std::uint16_t> {
public:
  static const VkIndexType value = VK_INDEX_TYPE_UINT16;
};

#ifdef VK_INDEX_TYPE_UINT8
/// @private
template <> class index_type<std::uint8_t> {
public:
  static const VkIndexType value = VK_INDEX_TYPE_UINT8;
};
#endif

class VulkanBuffer {
public:
  VulkanBuffer(VkDevice device, VmaAllocator allocator, VkBuffer buffer,
               VmaAllocation allocation, std::size_t count)
      : m_Device(device), m_Allocator(allocator), m_Buffer(buffer), m_Allocation(allocation), m_Count(count) {}

  VulkanBuffer(const VulkanBuffer &) = delete;
  VulkanBuffer &operator=(const VulkanBuffer &) = delete;

  ~VulkanBuffer() {
    vkDeviceWaitIdle(this->m_Device);
    vmaDestroyBuffer(this->m_Allocator, this->m_Buffer, this->m_Allocation);
  }

  /// @brief Returns a handle to the underlying `VkBuffer` object.
  const VkBuffer &handle() const { return this->m_Buffer; }

  /// @brief Returns a handle to the underlying `VkBuffer` object.
  VkBuffer &handle() { return this->m_Buffer; }

  /// @brief Returns the number of elements in the buffer.
  std::size_t count() const { return this->m_Count; }

  friend class Renderer;

private:
  VkDevice m_Device;
  VmaAllocator m_Allocator;
  VkBuffer m_Buffer;
  VmaAllocation m_Allocation;
  std::size_t m_Count;
};
} // namespace ashfault

#endif
