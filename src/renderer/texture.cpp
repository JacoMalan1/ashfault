#include <ashfault/renderer/texture.h>
#include <vulkan/vulkan_core.h>
#include <ashfault/renderer/buffer.hpp>

namespace ashfault {
VulkanTexture::VulkanTexture(std::pair<VkImage, VmaAllocation> image,
                             VkImageView image_view, VkDevice device, VmaAllocator allocator)
    : m_Image(image), m_ImageView(image_view), m_Device(device), m_Allocator(allocator) {}

VkImage &VulkanTexture::image() { return m_Image.first; }

VkImageView &VulkanTexture::image_view() { return m_ImageView; }

void VulkanTexture::destroy() {
  vkDestroyImageView(m_Device, m_ImageView, nullptr);
  vmaDestroyImage(m_Allocator, m_Image.first, m_Image.second);
}
}  // namespace ashfault
