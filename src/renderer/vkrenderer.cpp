#include <ashfault/renderer/descriptor_set.h>
#include <ashfault/renderer/pipeline.h>
#include <ashfault/renderer/shader.h>
#include <ashfault/renderer/swapchain.h>
#include <ashfault/renderer/vkrenderer.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <ashfault/renderer/buffer.hpp>
#include <cstdint>
#include <cstring>
#include <limits>
#include <set>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

QueueSuitability VulkanRenderer::find_queue_families(VkPhysicalDevice device) {
  std::uint32_t queue_family_count;
  std::vector<VkQueueFamilyProperties> queue_family_props;
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

bool VulkanRenderer::check_device_extension_support(VkPhysicalDevice device) {
  std::uint32_t extension_count;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       nullptr);
  std::vector<VkExtensionProperties> props;
  props.resize(extension_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       props.data());

  std::set<std::string> required_extensions = {this->s_DeviceExtensions.begin(),
                                               this->s_DeviceExtensions.end()};

  for (const auto& extension : props) {
    required_extensions.erase(extension.extensionName);
  }

  return required_extensions.empty();
}

bool VulkanRenderer::check_device_suitability(VkPhysicalDevice device) {
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

SwapchainSupportDetails VulkanRenderer::query_swapchain_support(
    VkPhysicalDevice device) {
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
VulkanRenderer::choose_physical_device() {
  std::uint32_t device_count;
  std::vector<VkPhysicalDevice> devices;
  vkEnumeratePhysicalDevices(this->m_Instance, &device_count, nullptr);
  devices.resize(device_count);
  vkEnumeratePhysicalDevices(this->m_Instance, &device_count, devices.data());

  std::sort(devices.begin(), devices.end(),
            [](VkPhysicalDevice a, VkPhysicalDevice b) {
              VkPhysicalDeviceProperties a_props;
              VkPhysicalDeviceProperties b_props;
              vkGetPhysicalDeviceProperties(a, &a_props);
              vkGetPhysicalDeviceProperties(b, &b_props);

              return device_type_ranking(b_props.deviceType) >
                     device_type_ranking(a_props.deviceType);
            });

  std::for_each(devices.begin(), devices.end(), [](VkPhysicalDevice device) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    SPDLOG_INFO("Found physical device: {}", props.deviceName);
  });

  for (std::size_t i = 0; i < device_count; i++) {
    check_device_suitability(devices[i]);
    QueueSuitability queue_info = find_queue_families(devices[i]);
    return std::make_optional(std::make_pair(devices[i], queue_info));
  }

  return {};
}

void ashfault::VulkanRenderer::create_instance() {
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  app_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
  app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
  app_info.pApplicationName = "AshFault";
  app_info.pEngineName = "AshFault";

  std::vector<const char*> enabled_layers = {};
  std::uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> layer_props = {};
  layer_props.resize(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, layer_props.data());

  for (std::size_t i = 0; i < layer_props.size(); i++) {
    if (std::strcmp(layer_props[i].layerName, "VK_LAYER_KHRONOS_validation") ==
        0) {
      enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
      SPDLOG_INFO("Enabling validation layers");
    }
  }

  std::vector<const char*> enabled_extensions =
      this->m_Window->required_instance_extensions();
  enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  std::for_each(enabled_extensions.begin(), enabled_extensions.end(),
                [](const char* name) {
                  SPDLOG_DEBUG("Required instance extension: {}", name);
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

void ashfault::VulkanRenderer::create_device() {
  auto [physical_device, queue_info] = choose_physical_device().value();
  this->m_QueueFamilies = queue_info;
  this->m_PhysicalDevice = physical_device;
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(physical_device, &props);
  SPDLOG_INFO("Picked physical device: {}", props.deviceName);

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
  SPDLOG_INFO("MSAA Samples: {}", (int)msaa_samples);

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

void ashfault::VulkanRenderer::create_allocator() {
  VmaAllocatorCreateInfo allocator_info{};
  allocator_info.device = this->m_Device;
  allocator_info.physicalDevice = this->m_PhysicalDevice;
  allocator_info.instance = this->m_Instance;
  allocator_info.vulkanApiVersion = VK_API_VERSION_1_3;

  vmaCreateAllocator(&allocator_info, &this->m_Allocator);
}

void ashfault::VulkanRenderer::create_surface() {
  this->m_Surface = this->m_Window->create_surface(this->m_Instance);
}

VkSurfaceFormatKHR VulkanRenderer::select_surface_format(
    const std::vector<VkSurfaceFormatKHR>& formats) {
  for (const auto& format : formats) {
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

VkPresentModeKHR VulkanRenderer::select_present_mode(
    const std::vector<VkPresentModeKHR>& formats) {
  auto preference_order = formats;
  std::sort(preference_order.begin(), preference_order.end(),
            [](VkPresentModeKHR a, VkPresentModeKHR b) {
              return present_mode_ranking(a) < present_mode_ranking(b);
            });

  return preference_order[0];
}

VkExtent2D VulkanRenderer::choose_swap_extent(
    VkSurfaceCapabilitiesKHR capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<std::uint32_t>::max()) {
    return capabilities.currentExtent;
  }

  WindowDims dims = this->m_Window->current_size();
  VkExtent2D extent = {dims.width, dims.height};

  extent.width = std::clamp(extent.width, capabilities.minImageExtent.width,
                            capabilities.maxImageExtent.width);
  extent.height = std::clamp(extent.height, capabilities.minImageExtent.height,
                             capabilities.maxImageExtent.height);

  return extent;
}

void ashfault::VulkanRenderer::setup_swapchain() {
  SwapchainSupportDetails swapchain_support =
      this->query_swapchain_support(this->m_PhysicalDevice);
  VkSurfaceFormatKHR swapchain_surface_format =
      select_surface_format(swapchain_support.formats);
  VkPresentModeKHR swapchain_present_mode =
      select_present_mode(swapchain_support.present_modes);
  SPDLOG_INFO("Selected present mode {}", (int)swapchain_present_mode);

  VkExtent2D swap_extent = choose_swap_extent(swapchain_support.capabilities);

  std::uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
  SPDLOG_INFO("Built swapchain with extent: {}, {}", swap_extent.width,
              swap_extent.height);
  this->m_Swapchain = new Swapchain(
      swapchain_surface_format, swapchain_present_mode, image_count,
      swap_extent, this->m_Surface, swapchain_support, this->m_Device);

  SPDLOG_DEBUG("Swapchain image count: {}", image_count);

  this->m_SurfaceFormat = swapchain_surface_format;
  this->m_SwapExtent = swap_extent;
  this->m_PresentMode = swapchain_present_mode;

  SPDLOG_INFO("Created swapchain");
}

void VulkanRenderer::setup_synchronization() {
  std::size_t image_count = std::max<std::size_t>(
      this->m_Swapchain->image_count(), MAX_FRAMES_IN_FLIGHT);

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

void VulkanRenderer::setup_command_buffers() {
  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.queueFamilyIndex = this->m_QueueFamilies.graphics_queue.value();
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(this->m_Device, &pool_info, nullptr,
                          &this->m_CommandPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create command pool");
  }
}

void VulkanRenderer::command_buffer(std::function<void(VkCommandBuffer)> op) {
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

void VulkanRenderer::setup_imgui() {
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  VkPipelineRenderingCreateInfo rendering_info{};
  rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  rendering_info.colorAttachmentCount = 1;
  rendering_info.pColorAttachmentFormats = &this->m_SurfaceFormat.format;
  rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

  ImGui_ImplGlfw_InitForVulkan(this->m_Window->handle(), true);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = this->m_Instance;
  init_info.PhysicalDevice = this->m_PhysicalDevice;
  init_info.Device = this->m_Device;
  init_info.QueueFamily = this->m_QueueFamilies.graphics_queue.value();
  init_info.Queue = this->m_GraphicsQueue;
  init_info.DescriptorPoolSize =
      IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
  init_info.MinImageCount = this->m_Swapchain->image_count();
  init_info.ImageCount = this->m_Swapchain->image_count();
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo = rendering_info;
  init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.UseDynamicRendering = true;
  ImGui_ImplVulkan_Init(&init_info);
}

void ashfault::VulkanRenderer::init(std::shared_ptr<Window> window) {
  this->m_Window = window;

  this->m_Window->set_resize_callback([&](Window& window, WindowDims) {
    this->m_Resized = true;
    this->recreate_swapchain();
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
  this->setup_imgui();
}

std::shared_ptr<VulkanShader> VulkanRenderer::create_shader(
    const std::string& path) const {
  return std::make_shared<VulkanShader>(this->m_Device, path);
}

GraphicsPipelineBuilder VulkanRenderer::create_graphics_pipeline() const {
  std::array<std::uint32_t, 2> window_dims;
  WindowDims dims = this->m_Window->current_size();
  window_dims[0] = dims.width;
  window_dims[1] = dims.height;
  return GraphicsPipelineBuilder(this->m_Device, this->m_SurfaceFormat.format,
                                 std::move(window_dims), this->m_MsaaSamples);
}

std::pair<VkImage, VmaAllocation> VulkanRenderer::create_image(
    uint32_t width, uint32_t height, VkSampleCountFlagBits samples,
    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
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

VkImageView VulkanRenderer::create_image_view(VkImage image, VkFormat format,
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

std::pair<VkBuffer, VmaAllocation> VulkanRenderer::create_buffer(
    std::size_t size, VkBufferUsageFlags usage,
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

void VulkanRenderer::copy_buffer(VkBuffer dst, VkBuffer src, std::size_t size) {
  VkBufferCopy region{};
  region.size = size;
  region.srcOffset = 0;
  region.dstOffset = 0;

  this->command_buffer(
      [&](VkCommandBuffer cmd) { vkCmdCopyBuffer(cmd, src, dst, 1, &region); });
}

void VulkanRenderer::recreate_swapchain() {
  SPDLOG_WARN("Rebuilding swapchain");
  WindowDims dims = this->m_Window->current_size();
  while (dims.width == 0 || dims.height == 0) {
    dims = this->m_Window->current_size();
    this->m_Window->wait_events();
  }

  vkDeviceWaitIdle(this->m_Device);
  this->cleanup_swapchain();
  this->setup_swapchain();

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
}

void VulkanRenderer::cleanup_swapchain() { this->m_Swapchain->cleanup(); }

void VulkanRenderer::shutdown() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  SPDLOG_INFO("Renderer shutting down...");

  vkDeviceWaitIdle(this->m_Device);
  vkQueueWaitIdle(this->m_GraphicsQueue);
  vkQueueWaitIdle(this->m_PresentQueue);

  this->cleanup_swapchain();

  vkDestroyCommandPool(this->m_Device, this->m_CommandPool, nullptr);

  for (std::size_t i = 0; i < this->m_ImageAvailableSemaphores.size(); i++) {
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

VulkanDescriptorSetBuilder VulkanRenderer::create_descriptor_sets() const {
  return VulkanDescriptorSetBuilder(this->m_Device);
}

VkDevice VulkanRenderer::device() { return this->m_Device; }

VmaAllocator VulkanRenderer::allocator() { return this->m_Allocator; }

std::vector<VkCommandBuffer> VulkanRenderer::allocate_command_buffers(
    std::uint32_t count) {
  std::vector<VkCommandBuffer> ret;
  ret.resize(count);

  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandBufferCount = count;
  alloc_info.commandPool = this->m_CommandPool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  VK_CHECK_RESULT(
      vkAllocateCommandBuffers(this->m_Device, &alloc_info, ret.data()));
  return ret;
}

Swapchain* VulkanRenderer::swapchain() { return this->m_Swapchain; }

VkSampler VulkanRenderer::create_sampler() {
  VkSamplerCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  create_info.anisotropyEnable = VK_FALSE;

  VkSampler ret;
  VK_CHECK_RESULT(vkCreateSampler(this->m_Device, &create_info, nullptr, &ret));
  return ret;
}

VkSampleCountFlagBits VulkanRenderer::msaa_samples() const {
  return this->m_MsaaSamples;
}

std::vector<VkSemaphore> VulkanRenderer::create_semaphores(std::size_t count) {
  std::vector<VkSemaphore> result{};
  result.resize(count);
  VkSemaphoreCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  for (std::size_t i = 0; i < count; i++) {
    VK_CHECK_RESULT(
        vkCreateSemaphore(m_Device, &create_info, nullptr, &result[i]));
  }

  return result;
}

std::vector<VkFence> VulkanRenderer::create_fences(std::size_t count) {
  std::vector<VkFence> result{};
  result.resize(count);
  VkFenceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (std::size_t i = 0; i < count; i++) {
    VK_CHECK_RESULT(vkCreateFence(m_Device, &create_info, nullptr, &result[i]));
  }

  return result;
}

VkQueue& VulkanRenderer::graphics_queue() { return m_GraphicsQueue; }

VkQueue& VulkanRenderer::present_queue() { return m_PresentQueue; }
}  // namespace ashfault
