#include <ashfault/renderer/renderer.h>
#include <ashfault/renderer/swapchain.h>
#include <limits>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace ashfault {
Swapchain::Swapchain(VkSurfaceFormatKHR format, VkPresentModeKHR present_mode,
                     std::uint32_t image_count, VkExtent2D extent,
                     VkSurfaceKHR surface, SwapchainSupportDetails support,
                     VkDevice device)
    : m_ImageCount(image_count), m_SwapExtent(extent), m_SurfaceFormat(format),
      m_PresentMode(present_mode), m_Surface(surface), m_Support(support),
      m_Device(device) {
  this->build();
}

void Swapchain::build() {
  VkSwapchainCreateInfoKHR swapchain_info{};
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface = this->m_Surface;
  swapchain_info.minImageCount = this->m_ImageCount;
  swapchain_info.imageFormat = this->m_SurfaceFormat.format;
  swapchain_info.imageColorSpace = this->m_SurfaceFormat.colorSpace;
  swapchain_info.imageExtent = this->m_SwapExtent;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_info.preTransform = this->m_Support.capabilities.currentTransform;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.presentMode = this->m_PresentMode;
  swapchain_info.clipped = true;
  swapchain_info.oldSwapchain = VK_NULL_HANDLE;

  VK_CHECK_RESULT(vkCreateSwapchainKHR(this->m_Device, &swapchain_info, nullptr,
                                       &this->m_Handle));

  this->m_Images.resize(this->m_ImageCount);
  VK_CHECK_RESULT(vkGetSwapchainImagesKHR(this->m_Device, this->m_Handle,
                                          &this->m_ImageCount,
                                          this->m_Images.data()));

  this->m_ImageViews.resize(this->m_ImageCount);
  for (std::size_t i = 0; i < this->m_ImageCount; i++) {
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = this->m_Images[i];
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.format = this->m_SurfaceFormat.format;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

    VK_CHECK_RESULT(vkCreateImageView(this->m_Device, &create_info, nullptr,
                                      &this->m_ImageViews[i]));
  }
}

void Swapchain::cleanup() {
  for (std::size_t i = 0; i < this->m_ImageCount; i++) {
    vkDestroyImageView(this->m_Device, this->m_ImageViews[i], nullptr);
  }

  vkDestroySwapchainKHR(this->m_Device, this->m_Handle, nullptr);
}

std::optional<std::uint32_t> Swapchain::acquire_image(VkSemaphore semaphore) {
  std::uint32_t ret;
  VkResult result = vkAcquireNextImageKHR(this->m_Device, this->m_Handle,
                        std::numeric_limits<std::uint64_t>::max(), semaphore,
                        VK_NULL_HANDLE, &ret);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    return {};
  }

  return ret;
}
} // namespace ashfault
