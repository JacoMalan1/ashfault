#include "ashfault/buffer.hpp"
#include "ashfault/descriptor_set.h"
#include "ashfault/frame.h"
#include "ashfault/pipeline.h"
#include "ashfault/shader.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <CLSTL/algorithm.h>
#include <CLSTL/array.h>
#include <CLSTL/vector.h>
#include <algorithm>
#include <ashfault/renderer.h>
#include <cstdint>
#include <cstring>
#include <limits>
#include <set>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace ashfault {

int device_type_ranking(VkPhysicalDeviceType type) {
  switch (type) {
  case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    return 0;
  case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
    return 1;
  case VK_PHYSICAL_DEVICE_TYPE_CPU:
    return 2;
  default:
    return 3;
  }
}

QueueSuitability Renderer::find_queue_families(VkPhysicalDevice device) {
  std::uint32_t queue_family_count;
  clstl::vector<VkQueueFamilyProperties> queue_family_props;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           nullptr);
  queue_family_props.resize(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           queue_family_props.data());

  QueueSuitability suitability{};

  for (std::size_t i = 0; i < queue_family_props.size(); i++) {
    if (queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      suitability.graphics_queue = i;
    }

    VkBool32 present_support;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->m_Surface,
                                         &present_support);
    if (present_support) {
      suitability.present_queue = i;
    }

    if (suitability.complete()) {
      break;
    }
  }

  return suitability;
}

bool Renderer::check_device_extension_support(VkPhysicalDevice device) {
  std::uint32_t extension_count;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       nullptr);
  std::vector<VkExtensionProperties> props;
  props.resize(extension_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       props.data());

  std::set<std::string> required_extensions = {this->s_DeviceExtensions.begin(),
                                               this->s_DeviceExtensions.end()};

  for (const auto &extension : props) {
    required_extensions.erase(extension.extensionName);
  }

  return required_extensions.empty();
}

bool Renderer::check_device_suitability(VkPhysicalDevice device) {
  QueueSuitability queue_families = find_queue_families(device);
  bool extension_support = check_device_extension_support(device);
  bool swapchain_adequate;

  if (extension_support) {
    SwapchainSupportDetails swapchain_details = query_swapchain_support(device);
    swapchain_adequate = !swapchain_details.formats.empty() &&
                         !swapchain_details.present_modes.empty();
  }

  return queue_families.complete() && extension_support && swapchain_adequate;
}

SwapchainSupportDetails
Renderer::query_swapchain_support(VkPhysicalDevice device) {
  SwapchainSupportDetails details{};

  std::uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->m_Surface, &format_count,
                                       nullptr);
  if (format_count > 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->m_Surface, &format_count,
                                         details.formats.data());
  }

  std::uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->m_Surface,
                                            &present_mode_count, nullptr);
  if (present_mode_count > 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->m_Surface,
                                              &present_mode_count,
                                              details.present_modes.data());
  }

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->m_Surface,
                                            &details.capabilities);

  return details;
}

std::optional<std::pair<VkPhysicalDevice, QueueSuitability>>
Renderer::choose_physical_device() {
  std::uint32_t device_count;
  clstl::vector<VkPhysicalDevice> devices;
  vkEnumeratePhysicalDevices(this->m_Instance, &device_count, nullptr);
  devices.resize(device_count);
  vkEnumeratePhysicalDevices(this->m_Instance, &device_count, devices.data());

  clstl::sort(devices.begin(), devices.end(),
              [](VkPhysicalDevice a, VkPhysicalDevice b) {
                VkPhysicalDeviceProperties a_props;
                VkPhysicalDeviceProperties b_props;
                vkGetPhysicalDeviceProperties(a, &a_props);
                vkGetPhysicalDeviceProperties(b, &b_props);

                return device_type_ranking(b_props.deviceType) >
                       device_type_ranking(a_props.deviceType);
              });

  clstl::for_each(devices.begin(), devices.end(), [](VkPhysicalDevice device) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    spdlog::info("Found physical device: {}", props.deviceName);
  });

  for (std::size_t i = 0; i < device_count; i++) {
    check_device_suitability(devices[i]);
    QueueSuitability queue_info = find_queue_families(devices[i]);
    return std::make_optional(std::make_pair(devices[i], queue_info));
  }

  return {};
}

void ashfault::Renderer::create_instance() {
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  app_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
  app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
  app_info.pApplicationName = "AshFault";
  app_info.pEngineName = "AshFault";

  clstl::vector<const char *> enabled_layers = {};
  std::uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> layer_props = {};
  layer_props.resize(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, layer_props.data());

  for (std::size_t i = 0; i < layer_props.size(); i++) {
    if (std::strcmp(layer_props[i].layerName, "VK_LAYER_KHRONOS_validation") ==
        0) {
      enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
      spdlog::info("Enabling validation layers");
    }
  }

  std::uint32_t extension_count;
  const char **required_extensions =
      glfwGetRequiredInstanceExtensions(&extension_count);
  clstl::vector<const char *> enabled_extensions;
  enabled_extensions.resize(extension_count);
  std::memcpy(enabled_extensions.data(), required_extensions,
              sizeof(const char *) * extension_count);
  enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  clstl::for_each(enabled_extensions.begin(), enabled_extensions.end(),
                  [](const char *name) {
                    spdlog::debug("Required instance extension: {}", name);
                  });

  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.ppEnabledLayerNames = enabled_layers.data();
  instance_info.enabledLayerCount = enabled_layers.size();
  instance_info.ppEnabledExtensionNames = enabled_extensions.data();
  instance_info.enabledExtensionCount = enabled_extensions.size();

  if (vkCreateInstance(&instance_info, nullptr, &this->m_Instance) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create vulkan instance");
  }
}

void ashfault::Renderer::create_device() {
  auto [physical_device, queue_info] = choose_physical_device().value();
  this->m_QueueFamilies = queue_info;
  this->m_PhysicalDevice = physical_device;
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(physical_device, &props);
  spdlog::info("Picked physical device: {}", props.deviceName);

  VkSampleCountFlags counts = props.limits.framebufferColorSampleCounts &
                              props.limits.framebufferDepthSampleCounts;

  VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_1_BIT;
  if (counts & VK_SAMPLE_COUNT_8_BIT)
    msaa_samples = VK_SAMPLE_COUNT_8_BIT;
  else if (counts & VK_SAMPLE_COUNT_4_BIT)
    msaa_samples = VK_SAMPLE_COUNT_4_BIT;
  else if (counts & VK_SAMPLE_COUNT_2_BIT)
    msaa_samples = VK_SAMPLE_COUNT_2_BIT;
  this->m_MsaaSamples = msaa_samples;
  spdlog::info("MSAA Samples: {}", (int)msaa_samples);

  float queue_prios = 1.0f;

  VkDeviceQueueCreateInfo graphics_queue_create_info{};
  graphics_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  graphics_queue_create_info.queueCount = 1;
  graphics_queue_create_info.queueFamilyIndex =
      queue_info.graphics_queue.value();
  graphics_queue_create_info.pQueuePriorities = &queue_prios;

  VkPhysicalDeviceVulkan12Features features_12{};
  features_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  features_12.runtimeDescriptorArray = VK_TRUE;

  VkPhysicalDeviceVulkan13Features features_13{};
  features_13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  features_13.dynamicRendering = VK_TRUE;
  features_13.pNext = &features_12;

  VkDeviceCreateInfo device_info{};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = &features_13;
  device_info.pQueueCreateInfos = &graphics_queue_create_info;
  device_info.queueCreateInfoCount = 1;
  device_info.enabledExtensionCount = this->s_DeviceExtensions.size();
  device_info.ppEnabledExtensionNames = this->s_DeviceExtensions.data();

  if (vkCreateDevice(physical_device, &device_info, nullptr, &this->m_Device) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan device");
  }

  vkGetDeviceQueue(this->m_Device, queue_info.graphics_queue.value(), 0,
                   &this->m_GraphicsQueue);
  vkGetDeviceQueue(this->m_Device, queue_info.present_queue.value(), 0,
                   &this->m_PresentQueue);
}

void ashfault::Renderer::create_allocator() {
  VmaAllocatorCreateInfo allocator_info{};
  allocator_info.device = this->m_Device;
  allocator_info.physicalDevice = this->m_PhysicalDevice;
  allocator_info.instance = this->m_Instance;
  allocator_info.vulkanApiVersion = VK_API_VERSION_1_3;

  vmaCreateAllocator(&allocator_info, &this->m_Allocator);
}

void ashfault::Renderer::create_surface() {
  if (glfwCreateWindowSurface(this->m_Instance, this->m_Window, nullptr,
                              &this->m_Surface) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create surface");
  }
}

VkSurfaceFormatKHR Renderer::select_surface_format(
    const clstl::vector<VkSurfaceFormatKHR> &formats) {
  for (const auto &format : formats) {
    if (format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR &&
        format.format == VK_FORMAT_B8G8R8A8_SRGB) {
      return format;
    }
  }

  return formats[0];
}

int present_mode_ranking(VkPresentModeKHR mode) {
  switch (mode) {
  case VK_PRESENT_MODE_MAILBOX_KHR:
    return 1;
  case VK_PRESENT_MODE_IMMEDIATE_KHR:
    return 2;
  case VK_PRESENT_MODE_FIFO_KHR:
    return 3;
  default:
    return 4;
  }
}

VkPresentModeKHR
Renderer::select_present_mode(const clstl::vector<VkPresentModeKHR> &formats) {
  auto preference_order = formats;
  clstl::sort(preference_order.begin(), preference_order.end(),
              [](VkPresentModeKHR a, VkPresentModeKHR b) {
                return present_mode_ranking(a) < present_mode_ranking(b);
              });

  return preference_order[0];
}

VkExtent2D Renderer::choose_swap_extent(VkSurfaceCapabilitiesKHR capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<std::uint32_t>::max()) {
    return capabilities.currentExtent;
  }

  int width, height;
  glfwGetFramebufferSize(this->m_Window, &width, &height);
  VkExtent2D extent = {static_cast<std::uint32_t>(width),
                       static_cast<std::uint32_t>(height)};

  extent.width = std::clamp(extent.width, capabilities.minImageExtent.width,
                            capabilities.maxImageExtent.width);
  extent.height = std::clamp(extent.height, capabilities.minImageExtent.height,
                             capabilities.maxImageExtent.height);

  return extent;
}

void ashfault::Renderer::setup_swapchain() {
  SwapchainSupportDetails swapchain_support =
      this->query_swapchain_support(this->m_PhysicalDevice);
  VkSurfaceFormatKHR swapchain_surface_format =
      select_surface_format(swapchain_support.formats);
  VkPresentModeKHR swapchain_present_mode =
      select_present_mode(swapchain_support.present_modes);
  spdlog::info("Selected present mode {}", (int)swapchain_present_mode);

  VkExtent2D swap_extent = choose_swap_extent(swapchain_support.capabilities);

  std::uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
  spdlog::debug("Swapchain image count: {}", image_count);
  VkSwapchainCreateInfoKHR swapchain_info{};
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface = this->m_Surface;
  swapchain_info.minImageCount = image_count;
  swapchain_info.imageFormat = swapchain_surface_format.format;
  swapchain_info.imageColorSpace = swapchain_surface_format.colorSpace;
  swapchain_info.imageExtent = swap_extent;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_info.preTransform = swapchain_support.capabilities.currentTransform;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.presentMode = swapchain_present_mode;
  swapchain_info.clipped = true;
  swapchain_info.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(this->m_Device, &swapchain_info, nullptr,
                           &this->m_Swapchain) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create swapchain");
  }

  this->m_SurfaceFormat = swapchain_surface_format;
  this->m_SwapExtent = swap_extent;
  this->m_PresentMode = swapchain_present_mode;

  std::uint32_t swapchain_image_count;
  vkGetSwapchainImagesKHR(this->m_Device, this->m_Swapchain,
                          &swapchain_image_count, nullptr);
  this->m_SwapchainImages.resize(swapchain_image_count);
  vkGetSwapchainImagesKHR(this->m_Device, this->m_Swapchain,
                          &swapchain_image_count,
                          this->m_SwapchainImages.data());

  this->m_ImageViews.resize(swapchain_image_count);
  for (std::size_t i = 0; i < this->m_SwapchainImages.size(); i++) {
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.format = swapchain_surface_format.format;
    create_info.image = this->m_SwapchainImages[i];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(this->m_Device, &create_info, nullptr,
                          &this->m_ImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create image views");
    }
  }

  spdlog::info("Created swapchain");
}

void Renderer::setup_synchronization() {
  std::size_t image_count = std::max<std::size_t>(
      this->m_SwapchainImages.size(), MAX_FRAMES_IN_FLIGHT);

  this->m_ImageAvailableSemaphores.resize(image_count);
  this->m_RenderFinishedSemaphores.resize(image_count);
  this->m_InFlightFences.resize(image_count);

  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (std::uint32_t i = 0; i < image_count; i++) {
    if (vkCreateSemaphore(this->m_Device, &semaphore_info, nullptr,
                          &this->m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(this->m_Device, &semaphore_info, nullptr,
                          &this->m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(this->m_Device, &fence_info, nullptr,
                      &this->m_InFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to setup synchronization");
    }
  }
}

void Renderer::setup_command_buffers() {
  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.queueFamilyIndex = this->m_QueueFamilies.graphics_queue.value();
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(this->m_Device, &pool_info, nullptr,
                          &this->m_CommandPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create command pool");
  }

  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
  alloc_info.commandPool = this->m_CommandPool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  this->m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateCommandBuffers(this->m_Device, &alloc_info,
                               this->m_CommandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers");
  }
}

void Renderer::command_buffer(std::function<void(VkCommandBuffer)> op) {
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandBufferCount = 1;
  alloc_info.commandPool = this->m_CommandPool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  VkCommandBuffer cmd;
  if (vkAllocateCommandBuffers(this->m_Device, &alloc_info, &cmd) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffer");
  }

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkResetCommandBuffer(cmd, 0);
  vkBeginCommandBuffer(cmd, &begin_info);
  op(cmd);
  vkEndCommandBuffer(cmd);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd;

  vkQueueSubmit(this->m_GraphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(this->m_GraphicsQueue);
  vkFreeCommandBuffers(this->m_Device, this->m_CommandPool, 1, &cmd);
}

void Renderer::create_color_resources() {
  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  auto [image, allocation] = this->create_image(
      this->m_SwapExtent.width, this->m_SwapExtent.height, this->m_MsaaSamples,
      this->m_SurfaceFormat.format, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      alloc_info);

  this->m_ColorImage = image;
  this->m_ColorImageAllocation = allocation;
  this->m_ColorImageView = this->create_image_view(
      image, this->m_SurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Renderer::create_depth_buffers() {
  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  auto [image, allocation] = this->create_image(
      this->m_SwapExtent.width, this->m_SwapExtent.height, this->m_MsaaSamples,
      VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, alloc_info);
  this->m_DepthImage = image;
  this->m_DepthImageAllocation = allocation;
  this->m_DepthImageView = this->create_image_view(image, VK_FORMAT_D32_SFLOAT,
                                                   VK_IMAGE_ASPECT_DEPTH_BIT);
}

std::optional<Frame> Renderer::start_frame() {
  vkWaitForFences(this->m_Device, 1,
                  &this->m_InFlightFences[this->m_CurrentFrame], VK_TRUE,
                  std::numeric_limits<std::uint64_t>::max());

  std::uint32_t image_i;
  VkResult result = vkAcquireNextImageKHR(
      this->m_Device, this->m_Swapchain,
      std::numeric_limits<std::uint64_t>::max(),
      this->m_ImageAvailableSemaphores[this->m_CurrentFrame], VK_NULL_HANDLE,
      &image_i);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    this->m_Resized = true;
    this->recreate_swapchain();
  }

  vkResetFences(this->m_Device, 1,
                &this->m_InFlightFences[this->m_CurrentFrame]);
  vkResetCommandBuffer(this->m_CommandBuffers[this->m_CurrentFrame], 0);
  VkCommandBuffer cmd = this->m_CommandBuffers[this->m_CurrentFrame];

  VkCommandBufferBeginInfo cmd_begin{};
  cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd, &cmd_begin);

  VkRenderingAttachmentInfo color_attachment{};
  color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  color_attachment.imageView = this->m_ColorImageView;
  color_attachment.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  color_attachment.resolveImageView = this->m_ImageViews[image_i];
  color_attachment.resolveImageLayout =
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;

  VkRenderingAttachmentInfo depth_attachment{};
  depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
  depth_attachment.imageView = this->m_DepthImageView;
  depth_attachment.clearValue.depthStencil = {1.0f, 0};

  VkRenderingInfo rendering_info{};
  rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  rendering_info.pColorAttachments = &color_attachment;
  rendering_info.pDepthAttachment = &depth_attachment;
  rendering_info.colorAttachmentCount = 1;
  rendering_info.layerCount = 1;
  rendering_info.viewMask = 0;
  rendering_info.renderArea.offset.x = 0;
  rendering_info.renderArea.offset.y = 0;
  rendering_info.renderArea.extent = this->m_SwapExtent;

  VkImageMemoryBarrier image_barrier{};
  image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  image_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  image_barrier.image = this->m_SwapchainImages[image_i];
  image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  image_barrier.subresourceRange.baseArrayLayer = 0;
  image_barrier.subresourceRange.layerCount = 1;
  image_barrier.subresourceRange.baseMipLevel = 0;
  image_barrier.subresourceRange.levelCount = 1;

  VkImageMemoryBarrier color_image_barrier{};
  color_image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  color_image_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  color_image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  color_image_barrier.image = this->m_ColorImage;
  color_image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  color_image_barrier.subresourceRange.baseArrayLayer = 0;
  color_image_barrier.subresourceRange.layerCount = 1;
  color_image_barrier.subresourceRange.baseMipLevel = 0;
  color_image_barrier.subresourceRange.levelCount = 1;

  VkImageMemoryBarrier depth_barrier{};
  depth_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  depth_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  depth_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
  depth_barrier.image = this->m_DepthImage;
  depth_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  depth_barrier.subresourceRange.baseArrayLayer = 0;
  depth_barrier.subresourceRange.layerCount = 1;
  depth_barrier.subresourceRange.baseMipLevel = 0;
  depth_barrier.subresourceRange.levelCount = 1;

  clstl::array<VkImageMemoryBarrier, 2> color_barriers = {color_image_barrier,
                                                          image_barrier};

  vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                       nullptr, 0, nullptr, color_barriers.size(),
                       color_barriers.data());
  vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0,
                       nullptr, 0, nullptr, 1, &depth_barrier);

  vkCmdBeginRendering(cmd, &rendering_info);
  int width, height;
  glfwGetFramebufferSize(this->m_Window, &width, &height);
  VkViewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = static_cast<float>(width);
  viewport.height = static_cast<float>(height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(cmd, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.extent.width = static_cast<std::uint32_t>(width);
  scissor.extent.height = static_cast<std::uint32_t>(height);
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  vkCmdSetScissor(cmd, 0, 1, &scissor);

  return Frame(this->m_Device, cmd, this->m_GraphicsQueue, this->m_PresentQueue,
               this->m_Swapchain, this->m_SwapchainImages[image_i],
               this->m_ColorImage, image_i, &this->m_CurrentFrame,
               this->m_ImageAvailableSemaphores[this->m_CurrentFrame],
               this->m_RenderFinishedSemaphores[image_i],
               this->m_InFlightFences[this->m_CurrentFrame], this);
}

void ashfault::Renderer::init(GLFWwindow *window) {
  this->m_Window = window;
  glfwSetWindowUserPointer(this->m_Window, this);
  glfwSetFramebufferSizeCallback(
      this->m_Window, [](GLFWwindow *window, int width, int height) {
        Renderer *renderer =
            reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
        if (renderer)
          renderer->m_Resized = true;
      });

  this->m_Resized = false;
  this->m_CurrentFrame = 0;
  this->create_instance();
  this->create_surface();
  this->create_device();
  this->create_allocator();
  this->setup_swapchain();
  this->setup_synchronization();
  this->setup_command_buffers();
  this->create_color_resources();
  this->create_depth_buffers();
}

clstl::shared_ptr<VulkanShader>
Renderer::create_shader(const clstl::string &path) const {
  return clstl::make_shared<VulkanShader>(this->m_Device, path);
}

GraphicsPipelineBuilder Renderer::create_graphics_pipeline() const {
  clstl::array<std::uint32_t, 2> window_dims;
  int width, height;
  glfwGetFramebufferSize(this->m_Window, &width, &height);
  window_dims[0] = width;
  window_dims[1] = height;
  return GraphicsPipelineBuilder(this->m_Device, this->m_SurfaceFormat.format,
                                 std::move(window_dims), this->m_MsaaSamples);
}

std::pair<VkImage, VmaAllocation>
Renderer::create_image(uint32_t width, uint32_t height,
                       VkSampleCountFlagBits samples, VkFormat format,
                       VkImageTiling tiling, VkImageUsageFlags usage,
                       VmaAllocationCreateInfo allocation_info) {
  VkImageCreateInfo image_info{};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.format = format;
  image_info.usage = usage;
  image_info.tiling = tiling;
  image_info.extent.width = width;
  image_info.extent.height = height;
  image_info.extent.depth = 1;
  image_info.arrayLayers = 1;
  image_info.mipLevels = 1;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.samples = samples;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkImage image;
  VmaAllocation allocation;
  if (vmaCreateImage(this->m_Allocator, &image_info, &allocation_info, &image,
                     &allocation, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create image");
  }

  return std::make_pair(image, allocation);
}

VkImageView Renderer::create_image_view(VkImage image, VkFormat format,
                                        VkImageAspectFlags imageAspect) {
  VkImageViewCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  create_info.format = format;
  create_info.image = image;
  create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  create_info.subresourceRange.aspectMask = imageAspect;
  create_info.subresourceRange.baseArrayLayer = 0;
  create_info.subresourceRange.layerCount = 1;
  create_info.subresourceRange.baseMipLevel = 0;
  create_info.subresourceRange.levelCount = 1;

  VkImageView view;
  if (vkCreateImageView(this->m_Device, &create_info, nullptr, &view) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create image view");
  }

  return view;
}

std::pair<VkBuffer, VmaAllocation>
Renderer::create_buffer(std::size_t size, VkBufferUsageFlags usage,
                        VmaAllocationCreateInfo alloc_info) {
  VkBufferCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  create_info.size = size;
  create_info.usage = usage;

  VkBuffer buffer;
  VmaAllocation allocation;
  if (vmaCreateBuffer(this->m_Allocator, &create_info, &alloc_info, &buffer,
                      &allocation, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create buffer");
  }

  return std::make_pair(buffer, allocation);
}

void Renderer::copy_buffer(VkBuffer dst, VkBuffer src, std::size_t size) {
  VkBufferCopy region{};
  region.size = size;
  region.srcOffset = 0;
  region.dstOffset = 0;

  this->command_buffer(
      [&](VkCommandBuffer cmd) { vkCmdCopyBuffer(cmd, src, dst, 1, &region); });
}

void Renderer::recreate_swapchain() {
  spdlog::warn("Rebuilding swapchain");
  int width = 0, height = 0;
  glfwGetFramebufferSize(this->m_Window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(this->m_Window, &width, &height);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(this->m_Device);
  this->cleanup_swapchain();
  this->setup_swapchain();
  this->create_color_resources();
  this->create_depth_buffers();
}

void Renderer::cleanup_swapchain() {
  for (const auto &image_view : this->m_ImageViews) {
    vkDestroyImageView(this->m_Device, image_view, nullptr);
  }

  vkDestroySwapchainKHR(this->m_Device, this->m_Swapchain, nullptr);
  vkDestroyImageView(this->m_Device, this->m_DepthImageView, nullptr);
  vkDestroyImageView(this->m_Device, this->m_ColorImageView, nullptr);
  vmaDestroyImage(this->m_Allocator, this->m_DepthImage,
                  this->m_DepthImageAllocation);
  vmaDestroyImage(this->m_Allocator, this->m_ColorImage,
                  this->m_ColorImageAllocation);
}

Renderer::~Renderer() {
  spdlog::info("Renderer shutting down...");
  glfwSetWindowUserPointer(this->m_Window, nullptr);
  glfwSetFramebufferSizeCallback(this->m_Window, nullptr);

  vkDeviceWaitIdle(this->m_Device);
  vkQueueWaitIdle(this->m_GraphicsQueue);
  vkQueueWaitIdle(this->m_PresentQueue);

  this->cleanup_swapchain();

  vkFreeCommandBuffers(this->m_Device, this->m_CommandPool,
                       this->m_CommandBuffers.size(),
                       this->m_CommandBuffers.data());
  vkDestroyCommandPool(this->m_Device, this->m_CommandPool, nullptr);

  for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(this->m_Device, this->m_ImageAvailableSemaphores[i],
                       nullptr);
    vkDestroySemaphore(this->m_Device, this->m_RenderFinishedSemaphores[i],
                       nullptr);
    vkDestroyFence(this->m_Device, this->m_InFlightFences[i], nullptr);
  }

  vmaDestroyAllocator(this->m_Allocator);
  vkDestroyDevice(this->m_Device, nullptr);
  vkDestroySurfaceKHR(this->m_Instance, this->m_Surface, nullptr);
  vkDestroyInstance(this->m_Instance, nullptr);
}

bool QueueSuitability::complete() const {
  return this->present_queue.has_value() && this->graphics_queue.has_value();
}

VulkanDescriptorSetBuilder Renderer::create_descriptor_sets() const {
  return VulkanDescriptorSetBuilder(this->m_Device);
}

VkDevice Renderer::device() { return this->m_Device; }

VmaAllocator Renderer::allocator() { return this->m_Allocator; }
} // namespace ashfault
