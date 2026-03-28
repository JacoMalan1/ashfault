#ifndef ASHFAULT_VULKAN_TEXTURE_H
#define ASHFAULT_VULKAN_TEXTURE_H

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <utility>

namespace ashfault {
class VulkanTexture {
public:
  VulkanTexture(std::pair<VkImage, VmaAllocation> image, VkImageView image_view, VkDevice device, VmaAllocator allocator);

  void destroy();

  VkImage &image();
  VkImageView &image_view();

private:
  std::pair<VkImage, VmaAllocation> m_Image;
  VkImageView m_ImageView;
  VkDevice m_Device;
  VmaAllocator m_Allocator;
};
}

#endif
