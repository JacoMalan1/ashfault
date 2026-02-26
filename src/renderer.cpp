#include <GLFW/glfw3.h>
#include <algorithm>
#include <array>
#include <ashfault/renderer.h>
#include <cstdint>
#include <cstring>
#include <spdlog/spdlog.h>
#include <iterator>
#include <stdexcept>
#include <vector>
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

QueueSuitability check_device_suitability(VkPhysicalDevice device) {
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

    if (queue_family_props[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
      suitability.transfer_queue = i;
    }
  }

  return suitability;
}

std::optional<std::pair<VkPhysicalDevice, QueueSuitability>>
choose_physical_device(VkInstance instance) {
  std::uint32_t device_count;
  std::vector<VkPhysicalDevice> devices;
  vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
  devices.resize(device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

  std::stable_sort(devices.begin(), devices.end(),
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
    spdlog::info("Found physical device: {}", props.deviceName);
  });

  std::vector<QueueSuitability> queue_suitability;
  queue_suitability.reserve(device_count);
  std::transform(
      devices.begin(), devices.end(), std::back_inserter(queue_suitability),
      [](VkPhysicalDevice device) { return check_device_suitability(device); });

  for (std::size_t i = 0; i < device_count; i++) {
    if (queue_suitability[i].graphics_queue.has_value() &&
        queue_suitability[i].transfer_queue.has_value()) {
      return std::make_optional(
          std::make_pair(devices[i], queue_suitability[i]));
    }
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

  std::array<const char *, 1> enabled_layers = {"VK_LAYER_KHRONOS_validation"};
  std::uint32_t extension_count;
  const char **required_extensions =
      glfwGetRequiredInstanceExtensions(&extension_count);
  std::vector<const char *> enabled_extensions;
  enabled_extensions.resize(extension_count);
  std::memcpy(enabled_extensions.data(), required_extensions,
              sizeof(const char *) * extension_count);

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
  auto [physical_device, queue_info] =
      choose_physical_device(this->m_Instance).value();
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

  VkDeviceQueueCreateInfo transfer_queue_create_info{};
  transfer_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  transfer_queue_create_info.queueCount = 1;
  transfer_queue_create_info.pQueuePriorities = &queue_prios;
  transfer_queue_create_info.queueFamilyIndex =
      queue_info.transfer_queue.value();

  std::array<VkDeviceQueueCreateInfo, 2> queue_create_info = {
      graphics_queue_create_info, transfer_queue_create_info};

  VkDeviceCreateInfo device_info{};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pQueueCreateInfos = queue_create_info.data();
  device_info.queueCreateInfoCount = 2;

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

void ashfault::Renderer::init(GLFWwindow *window) {
  this->create_instance();
  this->create_device();
  this->create_allocator();
}

Renderer::~Renderer() {
  vmaDestroyAllocator(this->m_Allocator);
  vkDestroyDevice(this->m_Device, nullptr);
  vkDestroyInstance(this->m_Instance, nullptr);
}
} // namespace ashfault
