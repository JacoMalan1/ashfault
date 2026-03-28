#ifndef ASHFAULT_VK_RENDERER_H
#define ASHFAULT_VK_RENDERER_H

#include <ashfault/ashfault.h>
#include <ashfault/core/window.h>
#include <ashfault/renderer/descriptor_set.h>
#include <ashfault/renderer/pipeline.h>
#include <ashfault/renderer/shader.h>
#include <ashfault/renderer/texture.h>
#include <vk_mem_alloc.h>

#include <ashfault/renderer/buffer.hpp>
#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>
#include <type_traits>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <format>

#define MAX_FRAMES_IN_FLIGHT 3
#define VK_CHECK_RESULT(x)                                                 \
  {                                                                        \
    VkResult __result = x;                                                 \
    if (__result != VK_SUCCESS) {                                          \
      throw std::runtime_error(std::format(                                \
          "Vulkan error {} in {}:{}", (int)__result, __FILE__, __LINE__)); \
    }                                                                      \
  }

#ifdef ASHFAULT_VK_VALIDATION
#include <spdlog/spdlog.h>
#endif

namespace ashfault {
class ASHFAULT_API Swapchain;

struct ASHFAULT_API QueueSuitability {
  std::optional<std::uint32_t> graphics_queue;
  std::optional<std::uint32_t> present_queue;

  bool complete() const;
};

struct ASHFAULT_API SwapchainSupportDetails {
  std::vector<VkPresentModeKHR> present_modes;
  std::vector<VkSurfaceFormatKHR> formats;
  VkSurfaceCapabilitiesKHR capabilities;
};

class ASHFAULT_API VulkanRenderer {
public:
  VulkanRenderer() = default;
  VulkanRenderer(const VulkanRenderer &) = delete;
  VulkanRenderer &operator=(const VulkanRenderer &) = delete;
  void shutdown();

  std::uint32_t image_index() const;

  VkCommandPool &command_pool();

  Swapchain *swapchain();
  std::vector<VkCommandBuffer> allocate_command_buffers(std::uint32_t count);

  /// @brief Initializes the renderer.
  void init(std::shared_ptr<Window> window);

  /// @brief Creates a vulkan shader module object.
  /// @param path Path to the pre-compiled SPIR-V shader binary.
  std::shared_ptr<VulkanShader> create_shader(const std::string &path) const;

  /// @brief Returns a graphics pipeline builder.
  GraphicsPipelineBuilder create_graphics_pipeline() const;

  /// @brief Returns a descriptor set builder.
  VulkanDescriptorSetBuilder create_descriptor_sets() const;

  /// @brief Creates and executes a one-time command buffer.
  ///
  /// Creates a command buffer and runs the provided operator with that command
  /// buffer.
  ///
  /// @note This function creates a one-time command buffer. Consequently, the
  /// command buffer should not be stored for later use.
  ///
  /// @warning Only `vkCmd` functions should be called on the commad buffer,
  /// since it will be initialized and ended correctly by renderer.
  ///
  /// @param op A function takes the command buffer as an argument and records
  /// to it.
  void command_buffer(std::function<void(VkCommandBuffer)> op);

  std::vector<VkSemaphore> create_semaphores(std::size_t count);
  std::vector<VkFence> create_fences(std::size_t count);

  VkQueue &graphics_queue();
  VkQueue &present_queue();

  /// @brief Creates a vulkan image object.
  ///
  /// @param width The width of the image in pixels.
  /// @param height The height of the image in pixels.
  /// @param format The pixel format of the image.
  /// @param tiling The image tiling mode.
  /// @param usage The intended image usage.
  /// @param allocation_info Information about how to create the image's
  /// underlying memory store.
  std::pair<VkImage, VmaAllocation> create_image(
      uint32_t width, uint32_t height, VkSampleCountFlagBits samples,
      VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
      VmaAllocationCreateInfo allocation_info);

  /// @brief Creates a vulkan image view from the supplied image.
  ///
  /// @note The argument passed to `format` should match the format used to
  /// create the image.
  VkImageView create_image_view(VkImage image, VkFormat format,
                                VkImageAspectFlags imageAspect);
  VulkanTexture create_texture(std::uint32_t width, std::uint32_t height,
                               const char *data);
  VkSampler create_sampler();

  /// @brief Creates a vulkan memory buffer.
  ///
  /// @param usage The intended buffer usage.
  /// @param alloc_info Information about how to allocate the buffer's backing
  /// storage.
  std::pair<VkBuffer, VmaAllocation> create_buffer(
      std::size_t size, VkBufferUsageFlags usage,
      VmaAllocationCreateInfo alloc_info);

  /// @brief Clenas up and rebuilds the swapchain.
  void recreate_swapchain();

  /// @brief Copy from one vulkan buffer into another.
  void copy_buffer(VkBuffer dst, VkBuffer src, std::size_t size);

  /// @brief Returns a handle to the render device.
  VkDevice device();

  /// @brief Returns a handle to the GPU memory allocator.
  VmaAllocator allocator();

  VkSampleCountFlagBits msaa_samples() const;

  std::array<std::uint32_t, 2> viewport_size() const;

  template <class T>
    requires std::is_trivially_copyable<T>::value &&
             (!std::is_pointer<T>::value) &&
             (!std::is_lvalue_reference<T>::value) &&
             (!std::is_rvalue_reference<T>::value)
  std::shared_ptr<VulkanBuffer> create_uniform_buffer(const T &data) {
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    auto [staging_buffer, staging_alloc] = this->create_buffer(
        sizeof(T), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, alloc_info);

    T *mapping;
    vmaMapMemory(this->m_Allocator, staging_alloc,
                 reinterpret_cast<void **>(&mapping));
    std::memcpy(mapping, &data, sizeof(T));
    vmaUnmapMemory(this->m_Allocator, staging_alloc);

    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    auto [buffer, allocation] = this->create_buffer(
        sizeof(T),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        alloc_info);

    this->copy_buffer(buffer, staging_buffer, sizeof(T));
    vmaDestroyBuffer(this->m_Allocator, staging_buffer, staging_alloc);

    return std::make_shared<VulkanBuffer>(this->m_Device, this->m_Allocator,
                                          buffer, allocation, 1);
  }

  /// @brief Creates an index buffer.
  ///
  /// @note This function can only be instantiated with unsigned integers of
  /// size 8, 16, and 32 bits.
  template <class T>

    requires std::is_integral<T>::value && std::is_unsigned<T>::value
  std::shared_ptr<VulkanBuffer> create_index_buffer(
      const std::vector<T> &indices) {
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    auto [staging_buffer, staging_alloc] =
        this->create_buffer(indices.size() * sizeof(T),
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, alloc_info);

    T *mapping;
    vmaMapMemory(this->m_Allocator, staging_alloc,
                 reinterpret_cast<void **>(&mapping));
    std::memcpy(mapping, indices.data(), indices.size() * sizeof(T));
    vmaUnmapMemory(this->m_Allocator, staging_alloc);

    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    auto [buffer, allocation] = this->create_buffer(
        indices.size() * sizeof(T),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        alloc_info);

    this->copy_buffer(buffer, staging_buffer, indices.size() * sizeof(T));
    vmaDestroyBuffer(this->m_Allocator, staging_buffer, staging_alloc);

    return std::make_shared<VulkanBuffer>(this->m_Device, this->m_Allocator,
                                          buffer, allocation, indices.size());
  }

  /// @brief Creates a vertex buffer.
  template <class T>
    requires std::is_trivially_copyable<T>::value &&
             (!std::is_pointer<T>::value) &&
             (!std::is_lvalue_reference<T>::value) &&
             (!std::is_rvalue_reference<T>::value)
  std::shared_ptr<VulkanBuffer> create_vertex_buffer(
      const std::vector<T> &vertices) {
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    auto [staging_buffer, staging_alloc] =
        this->create_buffer(vertices.size() * sizeof(T),
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, alloc_info);

    T *mapping;
    vmaMapMemory(this->m_Allocator, staging_alloc,
                 reinterpret_cast<void **>(&mapping));
    std::memcpy(mapping, vertices.data(), vertices.size() * sizeof(T));
    vmaUnmapMemory(this->m_Allocator, staging_alloc);

    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    auto [buffer, allocation] = this->create_buffer(
        vertices.size() * sizeof(T),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        alloc_info);

    this->copy_buffer(buffer, staging_buffer, vertices.size() * sizeof(T));
    vmaDestroyBuffer(this->m_Allocator, staging_buffer, staging_alloc);

    return std::make_shared<VulkanBuffer>(this->m_Device, this->m_Allocator,
                                          buffer, allocation, vertices.size());
  }

private:
  const std::vector<const char *> s_DeviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME};

  std::shared_ptr<Window> m_Window;
  VkInstance m_Instance;
  VkPhysicalDevice m_PhysicalDevice;
  VkDevice m_Device;
  VmaAllocator m_Allocator;
  VkSurfaceKHR m_Surface;
  VkSurfaceFormatKHR m_SurfaceFormat;
  VkPresentModeKHR m_PresentMode;
  VkExtent2D m_SwapExtent;
  VkQueue m_GraphicsQueue;
  VkQueue m_PresentQueue;
  std::vector<VkSemaphore> m_ImageAvailableSemaphores;
  std::vector<VkSemaphore> m_RenderFinishedSemaphores;
  std::vector<VkFence> m_InFlightFences;
  std::uint32_t m_CurrentFrame;
  QueueSuitability m_QueueFamilies;
  VkCommandPool m_CommandPool;
  VkSampleCountFlagBits m_MsaaSamples;

  Swapchain *m_Swapchain;

  bool m_Resized;

  void create_instance();
  void setup_debug_callback();
  void create_surface();
  void create_device();
  void create_allocator();
  void setup_swapchain(bool vsync = false);
  void setup_synchronization();
  void setup_command_buffers();
  void cleanup_swapchain();
  void setup_imgui();

  VkSurfaceFormatKHR select_surface_format(
      const std::vector<VkSurfaceFormatKHR> &);
  VkPresentModeKHR select_present_mode(const std::vector<VkPresentModeKHR> &);
  VkExtent2D choose_swap_extent(VkSurfaceCapabilitiesKHR caps);

  bool check_device_suitability(VkPhysicalDevice device);
  QueueSuitability find_queue_families(VkPhysicalDevice device);
  bool check_device_extension_support(VkPhysicalDevice device);
  std::optional<std::pair<VkPhysicalDevice, QueueSuitability>>
  choose_physical_device();
  SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice device);

#ifdef ASHFAULT_VK_VALIDATION
  std::shared_ptr<spdlog::logger> m_VkDebugLogger;
  VkDebugUtilsMessengerEXT m_DebugMessenger;
#endif
};
}  // namespace ashfault

#endif
