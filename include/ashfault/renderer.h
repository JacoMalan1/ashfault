#ifndef ASHFAULT_RENDERER_H
#define ASHFAULT_RENDERER_H

#include "ashfault/buffer.hpp"
#include "ashfault/descriptor_set.h"
#include "ashfault/frame.h"
#include "ashfault/pipeline.h"
#include <CLSTL/shared_ptr.h>
#include <CLSTL/string.h>
#include <CLSTL/vector.h>
#include <ashfault/shader.h>
#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>
#include <vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define MAX_FRAMES_IN_FLIGHT 3

namespace ashfault {
struct QueueSuitability {
  std::optional<std::uint32_t> graphics_queue;
  std::optional<std::uint32_t> present_queue;

  bool complete() const;
};

struct SwapchainSupportDetails {
  clstl::vector<VkPresentModeKHR> present_modes;
  clstl::vector<VkSurfaceFormatKHR> formats;
  VkSurfaceCapabilitiesKHR capabilities;
};

class Renderer {
public:
  Renderer() = default;
  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;
  ~Renderer();
  friend class Frame;

  /// @brief Initializes the renderer.
  void init(GLFWwindow *window);

  /// @brief Creates a vulkan shader module object.
  /// @param path Path to the pre-compiled SPIR-V shader binary.
  clstl::shared_ptr<VulkanShader>
  create_shader(const clstl::string &path) const;

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

  /// @brief Creates a vulkan image object.
  ///
  /// @param width The width of the image in pixels.
  /// @param height The height of the image in pixels.
  /// @param format The pixel format of the image.
  /// @param tiling The image tiling mode.
  /// @param usage The intended image usage.
  /// @param allocation_info Information about how to create the image's
  /// underlying memory store.
  std::pair<VkImage, VmaAllocation>
  create_image(uint32_t width, uint32_t height, VkSampleCountFlagBits samples, VkFormat format,
               VkImageTiling tiling, VkImageUsageFlags usage,
               VmaAllocationCreateInfo allocation_info);

  /// @brief Creates a vulkan image view from the supplied image.
  ///
  /// @note The argument passed to `format` should match the format used to
  /// create the image.
  VkImageView create_image_view(VkImage image, VkFormat format,
                                VkImageAspectFlags imageAspect);

  /// @brief Creates a vulkan memory buffer.
  ///
  /// @param usage The intended buffer usage.
  /// @param alloc_info Information about how to allocate the buffer's backing
  /// storage.
  std::pair<VkBuffer, VmaAllocation>
  create_buffer(std::size_t size, VkBufferUsageFlags usage,
                VmaAllocationCreateInfo alloc_info);

  /// @brief Start recording a graphics queue command buffer.
  ///
  /// Acquires an image from the swapchain and starts recording a
  /// command buffer for rendering.
  std::optional<Frame> start_frame();

  /// @brief Clenas up and rebuilds the swapchain.
  void recreate_swapchain();

  /// @brief Copy from one vulkan buffer into another.
  void copy_buffer(VkBuffer dst, VkBuffer src, std::size_t size);

  /// @brief Returns a handle to the render device.
  VkDevice device();

  /// @brief Returns a handle to the GPU memory allocator.
  VmaAllocator allocator();

  /// @brief Creates an index buffer.
  ///
  /// @note This function can only be instantiated with unsigned integers of
  /// size 8, 16, and 32 bits.
  template <class T>
  clstl::shared_ptr<VulkanBuffer<T>>
  create_index_buffer(const clstl::vector<T> &indices) {
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);

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

    return clstl::make_shared<VulkanBuffer<T>>(
        this->m_Device, this->m_Allocator, buffer, allocation, indices.size());
  }

  /// @brief Creates a vertex buffer.
  template <class T>
  clstl::shared_ptr<VulkanBuffer<T>>
  create_vertex_buffer(const clstl::vector<T> &vertices) {
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

    return clstl::make_shared<VulkanBuffer<T>>(
        this->m_Device, this->m_Allocator, buffer, allocation, vertices.size());
  }

private:
  const std::vector<const char *> s_DeviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  GLFWwindow *m_Window;
  VkInstance m_Instance;
  VkPhysicalDevice m_PhysicalDevice;
  VkDevice m_Device;
  VmaAllocator m_Allocator;
  VkSurfaceKHR m_Surface;
  VkSwapchainKHR m_Swapchain;
  VkSurfaceFormatKHR m_SurfaceFormat;
  VkPresentModeKHR m_PresentMode;
  VkExtent2D m_SwapExtent;
  clstl::vector<VkImage> m_SwapchainImages;
  clstl::vector<VkImageView> m_ImageViews;
  VkQueue m_GraphicsQueue;
  VkQueue m_PresentQueue;
  clstl::vector<VkSemaphore> m_ImageAvailableSemaphores;
  clstl::vector<VkSemaphore> m_RenderFinishedSemaphores;
  clstl::vector<VkFence> m_InFlightFences;
  std::uint32_t m_CurrentFrame;
  clstl::vector<VkCommandBuffer> m_CommandBuffers;
  QueueSuitability m_QueueFamilies;
  VkCommandPool m_CommandPool;
  VkSampleCountFlagBits m_MsaaSamples;

  VkImage m_DepthImage;
  VmaAllocation m_DepthImageAllocation;
  VkImageView m_DepthImageView;
  VkImage m_ColorImage;
  VmaAllocation m_ColorImageAllocation;
  VkImageView m_ColorImageView;

  bool m_Resized;

  void create_instance();
  void create_surface();
  void create_device();
  void create_allocator();
  void setup_swapchain();
  void setup_synchronization();
  void setup_command_buffers();
  void create_depth_buffers();
  void create_color_resources();
  void cleanup_swapchain();

  VkSurfaceFormatKHR
  select_surface_format(const clstl::vector<VkSurfaceFormatKHR> &);
  VkPresentModeKHR select_present_mode(const clstl::vector<VkPresentModeKHR> &);
  VkExtent2D choose_swap_extent(VkSurfaceCapabilitiesKHR caps);

  bool check_device_suitability(VkPhysicalDevice device);
  QueueSuitability find_queue_families(VkPhysicalDevice device);
  bool check_device_extension_support(VkPhysicalDevice device);
  std::optional<std::pair<VkPhysicalDevice, QueueSuitability>>
  choose_physical_device();
  SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice device);
};
} // namespace ashfault

#endif
