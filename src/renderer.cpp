#include <algorithm>
#include <limits>
#include <set>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <CLSTL/algorithm.h>
#include <CLSTL/array.h>
#include <CLSTL/vector.h>
#include <ashfault/renderer.h>
#include <cstdint>
#include <cstring>
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
  this->m_PhysicalDevice = physical_device;
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(physical_device, &props);
  spdlog::info("Picked physical device: {}", props.deviceName);

  float queue_prios = 1.0f;

  VkDeviceQueueCreateInfo graphics_queue_create_info{};
  graphics_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  graphics_queue_create_info.queueCount = 1;
  graphics_queue_create_info.queueFamilyIndex =
      queue_info.graphics_queue.value();
  graphics_queue_create_info.pQueuePriorities = &queue_prios;

  VkDeviceQueueCreateInfo present_queue_create_info{};
  present_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  present_queue_create_info.queueCount = 1;
  present_queue_create_info.pQueuePriorities = &queue_prios;
  present_queue_create_info.queueFamilyIndex = queue_info.present_queue.value();

  clstl::array<VkDeviceQueueCreateInfo, 2> queue_create_info = {
      graphics_queue_create_info, present_queue_create_info};

  VkDeviceCreateInfo device_info{};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pQueueCreateInfos = queue_create_info.data();
  device_info.queueCreateInfoCount = 2;
  device_info.enabledExtensionCount = this->s_DeviceExtensions.size();
  device_info.ppEnabledExtensionNames = this->s_DeviceExtensions.data();

  if (vkCreateDevice(physical_device, &device_info, nullptr, &this->m_Device) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan device");
  }
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
  default:
    return 3;
  }
}

VkPresentModeKHR
Renderer::select_present_mode(const clstl::vector<VkPresentModeKHR> &formats) {
  auto preference_order = formats;
  clstl::sort(preference_order.begin(), preference_order.end(),
              [](VkPresentModeKHR a, VkPresentModeKHR b) {
                return present_mode_ranking(a) > present_mode_ranking(b);
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
  VkExtent2D swap_extent = choose_swap_extent(swapchain_support.capabilities);

  std::uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
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
                          &swapchain_image_count, this->m_SwapchainImages.data());

  this->m_ImageViews.resize(swapchain_image_count);
  for (std::size_t i = 0; i < this->m_SwapchainImages.size(); i++) {
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
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
}

void ashfault::Renderer::init(GLFWwindow *window) {
  this->m_Window = window;
  this->create_instance();
  this->create_surface();
  this->create_device();
  this->create_allocator();
  this->setup_swapchain();
}

Renderer::~Renderer() {
  for (const auto &image_view : this->m_ImageViews) {
    vkDestroyImageView(this->m_Device, image_view, nullptr);
  }

  vkDestroySwapchainKHR(this->m_Device, this->m_Swapchain, nullptr);

  vkDestroySurfaceKHR(this->m_Instance, this->m_Surface, nullptr);
  vmaDestroyAllocator(this->m_Allocator);
  vkDestroyDevice(this->m_Device, nullptr);
  vkDestroyInstance(this->m_Instance, nullptr);
}

bool QueueSuitability::complete() const {
  return this->present_queue.has_value() && this->graphics_queue.has_value();
}
} // namespace ashfault
