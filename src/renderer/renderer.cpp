#include <ashfault/renderer/buffer.hpp>
#include <ashfault/renderer/renderer.h>
#include <ashfault/renderer/swapchain.h>
#include <ashfault/renderer/vkrenderer.h>
#include <imgui_impl_vulkan.h>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace ashfault {
struct RendererData {
  std::shared_ptr<VulkanRenderer> render_backend;
  Swapchain *swapchain;

  std::vector<VkCommandBuffer> command_buffers;
  std::vector<VkCommandBuffer> imgui_command_buffers;

  std::uint32_t image_index;

  std::shared_ptr<RenderTarget> default_target;
  std::vector<std::shared_ptr<RenderTarget>> targets;
};

static RendererData s_Data;

void Renderer::start_frame() { s_Data.targets.clear(); }

void Renderer::end_frame() {}

void Renderer::init(std::shared_ptr<Window> window) {
  s_Data.render_backend = std::make_shared<VulkanRenderer>();
  s_Data.render_backend->init(window);
  s_Data.swapchain = s_Data.render_backend->swapchain();

  s_Data.imgui_command_buffers =
      s_Data.render_backend->allocate_command_buffers(
          s_Data.swapchain->image_count());
  s_Data.command_buffers = s_Data.render_backend->allocate_command_buffers(
      s_Data.swapchain->image_count());
}

void Renderer::submit_imgui_data(ImDrawData *draw_data) {
  if (draw_data)
    ImGui_ImplVulkan_RenderDrawData(
        draw_data, s_Data.imgui_command_buffers[s_Data.image_index]);
}

void Renderer::push_render_target(std::shared_ptr<RenderTarget> target) {
  s_Data.targets.push_back(target);
}

void Renderer::pop_render_target() {
  if (!s_Data.targets.empty())
    s_Data.targets.pop_back();
}

RenderTarget &Renderer::render_target() {
  return *(s_Data.targets.empty() ? s_Data.default_target
                                  : s_Data.targets.back());
}

std::shared_ptr<RenderTarget> Renderer::create_render_target(bool msaa) {
  auto image_count = s_Data.swapchain->image_count();
  auto dims = s_Data.swapchain->swap_extent();
  auto cmd_buffers =
      s_Data.render_backend->allocate_command_buffers(image_count);

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
  alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  std::vector<VkImage> images(image_count);
  std::vector<VmaAllocation> allocs(image_count);
  std::vector<VkImageView> views(image_count);

  std::pair<VkImage, VmaAllocation> depth_image =
      s_Data.render_backend->create_image(
          dims.width, dims.height,
          msaa ? s_Data.render_backend->msaa_samples() : VK_SAMPLE_COUNT_1_BIT,
          VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, alloc_info);
  VkImageView depth_view = s_Data.render_backend->create_image_view(
      depth_image.first, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);

  for (std::size_t i = 0; i < image_count; i++) {
    auto image = s_Data.render_backend->create_image(
        dims.width, dims.height,
        msaa ? s_Data.render_backend->msaa_samples() : VK_SAMPLE_COUNT_1_BIT,
        s_Data.swapchain->surface_format().format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, alloc_info);
    auto view = s_Data.render_backend->create_image_view(
        image.first, s_Data.swapchain->surface_format().format,
        VK_IMAGE_ASPECT_COLOR_BIT);
    images[i] = image.first;
    allocs[i] = image.second;
    views[i] = view;
  }

  return std::make_shared<RenderTarget>(
      s_Data.render_backend, depth_image, depth_view, images, views,
      std::make_optional(allocs), cmd_buffers);
}

void Renderer::submit_and_wait() {}
} // namespace ashfault
