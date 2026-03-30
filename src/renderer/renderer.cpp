#include <ashfault/core/camera.h>
#include <ashfault/core/material.h>
#include <ashfault/core/pipeline_manager.h>
#include <ashfault/renderer/descriptor_set.h>
#include <ashfault/renderer/pipeline.h>
#include <ashfault/renderer/renderer.h>
#include <ashfault/renderer/swapchain.h>
#include <ashfault/renderer/vkrenderer.h>
#include <ashfault/renderer/light.h>
#include <imgui_impl_vulkan.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <ashfault/renderer/buffer.hpp>
#include <cstddef>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <limits>
#include <memory>

namespace ashfault {
struct PushConstants {
  glm::mat4 projection;
  glm::mat4 view;
  glm::mat4 model;
  int albedo_texture_index, normal_texture_index;
};

struct RendererData {
  std::shared_ptr<VulkanRenderer> render_backend;
  Swapchain *swapchain;
  std::unique_ptr<PipelineManager> pipeline_manager;

  std::vector<VkSemaphore> image_available_semaphores;
  std::vector<VkSemaphore> render_finished_semaphores;
  std::vector<VkFence> in_flight_fences;
  VkPipelineStageFlags wait_stages;

  std::vector<VkCommandBuffer> cmd_to_submit;

  std::uint32_t image_index;
  std::uint32_t current_frame;

  std::shared_ptr<RenderTarget> default_target;
  std::vector<std::shared_ptr<RenderTarget>> targets;

  std::optional<PushConstants> camera_data;
  VkPipelineLayout pipeline_layout;
  std::vector<VkDescriptorSetLayout> push_descriptor_layouts;
  std::pair<VkBuffer, VmaAllocation> light_buffer;
  void *light_buffer_mapping;
  std::array<Light, ASHFAULT_MAX_LIGHTS> lights;
  std::int32_t light_count;

  std::vector<VkDescriptorSetLayout> texture_layouts;
  std::vector<VulkanTexture> textures;
  VkDescriptorSet texture_descriptors;
  VkDescriptorPool texture_pool;
  VkSampler sampler;
};

static RendererData s_Data;
static PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR;

void Renderer::create_descriptors() {
  s_Data.sampler = s_Data.render_backend->create_sampler();

  std::array<VkDescriptorPoolSize, 1> sizes{};
  sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sizes[0].descriptorCount = ASHFAULT_MAX_TEXTURES;

  VkDescriptorPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.pPoolSizes = sizes.data();
  pool_info.poolSizeCount = sizes.size();
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT |
                    VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1;

  VK_CHECK_RESULT(vkCreateDescriptorPool(s_Data.render_backend->device(),
                                         &pool_info, nullptr,
                                         &s_Data.texture_pool));

  VkDescriptorSetAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = s_Data.texture_pool;
  alloc_info.descriptorSetCount = 1;
  alloc_info.pSetLayouts = &s_Data.texture_layouts.front();

  VK_CHECK_RESULT(vkAllocateDescriptorSets(s_Data.render_backend->device(),
                                           &alloc_info,
                                           &s_Data.texture_descriptors));
}

void Renderer::create_pipelines() {
  std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
  bindings[0] = {.binding = 0,
                 .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                 .descriptorCount = 1,
                 .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT};

  VkDescriptorSetLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = bindings.size();
  layout_info.pBindings = bindings.data();
  layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

  s_Data.push_descriptor_layouts.resize(bindings.size());
  VK_CHECK_RESULT(vkCreateDescriptorSetLayout(
      s_Data.render_backend->device(), &layout_info, nullptr,
      s_Data.push_descriptor_layouts.data()));

  VkDescriptorSetLayoutBinding binding{};
  binding.descriptorCount = ASHFAULT_MAX_TEXTURES;
  binding.binding = 0;
  binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorBindingFlags binding_flags =
      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

  VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info{};
  binding_flags_info.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
  binding_flags_info.pBindingFlags = &binding_flags;
  binding_flags_info.bindingCount = 1;

  VkDescriptorSetLayoutCreateInfo texture_layout_info{};
  texture_layout_info.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  texture_layout_info.bindingCount = 1;
  texture_layout_info.pBindings = &binding;
  texture_layout_info.pNext = &binding_flags_info;

  s_Data.texture_layouts.resize(1);
  VK_CHECK_RESULT(vkCreateDescriptorSetLayout(s_Data.render_backend->device(),
                                              &texture_layout_info, nullptr,
                                              s_Data.texture_layouts.data()));

  std::vector<VkDescriptorSetLayout> all_layouts{};
  all_layouts.push_back(s_Data.push_descriptor_layouts[0]);
  all_layouts.push_back(s_Data.texture_layouts[0]);

  auto static_vshader =
      s_Data.render_backend->create_shader("blinn_phong.vert.spv");
  auto static_fshader =
      s_Data.render_backend->create_shader("blinn_phong.frag.spv");

  std::vector<VkVertexInputAttributeDescription> vertex_attribs{};
  vertex_attribs.push_back({
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
      .offset = 0,
  });
  vertex_attribs.push_back({
      .location = 1,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = offsetof(Mesh::Vertex, position),
  });
  vertex_attribs.push_back({
      .location = 2,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = offsetof(Mesh::Vertex, normal),
  });
  vertex_attribs.push_back({
      .location = 3,
      .binding = 0,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = offsetof(Mesh::Vertex, uv),
  });

  auto static_pipeline =
      s_Data.render_backend->create_graphics_pipeline()
          .input_attribute_descriptions(vertex_attribs, sizeof(Mesh::Vertex))
          .vertex_shader(static_vshader)
          .fragment_shader(static_fshader)
          .push_constant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants))
          .build(all_layouts);

  s_Data.pipeline_manager->add_graphics_pipeline("blinn_phong",
                                                 static_pipeline);
  s_Data.pipeline_layout = static_pipeline->layout();
}

bool Renderer::start_frame() {
  s_Data.swapchain = s_Data.render_backend->swapchain();
  s_Data.default_target->destroy();
  s_Data.default_target =
      create_render_target(s_Data.swapchain->swap_extent().width,
                           s_Data.swapchain->swap_extent().height, false, true);
  s_Data.cmd_to_submit.clear();
  vkWaitForFences(s_Data.render_backend->device(), 1,
                  &s_Data.in_flight_fences[s_Data.current_frame], VK_TRUE,
                  std::numeric_limits<std::uint64_t>::max());

  auto idx = s_Data.swapchain->acquire_image(
      s_Data.image_available_semaphores[s_Data.current_frame]);

  if (!idx.has_value()) {
    s_Data.render_backend->recreate_swapchain();
    return false;
  }
  vkResetFences(s_Data.render_backend->device(), 1,
                &s_Data.in_flight_fences[s_Data.current_frame]);

  s_Data.image_index = idx.value();
  s_Data.targets.clear();
  s_Data.wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkCommandBuffer cmd =
      s_Data.default_target->command_buffer(s_Data.current_frame);
  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd, &begin_info);

  s_Data.default_target->begin_rendering(s_Data.image_index,
                                         s_Data.current_frame);

  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          s_Data.pipeline_layout, 1, 1,
                          &s_Data.texture_descriptors, 0, nullptr);

  std::memcpy(s_Data.light_buffer_mapping, s_Data.lights.data(),
              sizeof(Light) * s_Data.lights.size());
  std::memcpy(reinterpret_cast<char *>(s_Data.light_buffer_mapping) +
                  sizeof(Light) * ASHFAULT_MAX_LIGHTS,
              &s_Data.light_count, sizeof(std::uint32_t));

  VkDescriptorBufferInfo buffer_info{};
  buffer_info.buffer = s_Data.light_buffer.first;
  buffer_info.offset = 0;
  buffer_info.range = sizeof(Light) * ASHFAULT_MAX_LIGHTS + sizeof(int);

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  write.dstBinding = 0;
  write.pBufferInfo = &buffer_info;
  vkCmdPushDescriptorSetKHR(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            s_Data.pipeline_layout, 0, 1, &write);
  return true;
}

void Renderer::end_frame() {
  VkCommandBuffer cmd =
      s_Data.default_target->command_buffer(s_Data.current_frame);

  s_Data.default_target->end_rendering(s_Data.image_index, s_Data.current_frame,
                                       true);
  vkEndCommandBuffer(cmd);
  s_Data.cmd_to_submit.push_back(
      s_Data.default_target->command_buffer(s_Data.current_frame));
}

void Renderer::init(std::shared_ptr<Window> window) {
  s_Data.render_backend = std::make_shared<VulkanRenderer>();
  s_Data.render_backend->init(window);

  vkCmdPushDescriptorSetKHR =
      (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(
          s_Data.render_backend->device(), "vkCmdPushDescriptorSetKHR");
  SPDLOG_DEBUG("Loaded `vkCmdPushDescriptorSetKHR` ({})",
               (void *)vkCmdPushDescriptorSetKHR);

  s_Data.swapchain = s_Data.render_backend->swapchain();
  s_Data.pipeline_manager = std::make_unique<PipelineManager>();
  s_Data.current_frame = 0;
  s_Data.default_target = create_render_target(
      s_Data.render_backend->swapchain()->swap_extent().width,
      s_Data.render_backend->swapchain()->swap_extent().height, false, true);
  s_Data.cmd_to_submit = {};

  s_Data.image_available_semaphores =
      s_Data.render_backend->create_semaphores(s_Data.swapchain->image_count());
  s_Data.render_finished_semaphores =
      s_Data.render_backend->create_semaphores(s_Data.swapchain->image_count());
  s_Data.in_flight_fences =
      s_Data.render_backend->create_fences(s_Data.swapchain->image_count());

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                     VMA_ALLOCATION_CREATE_MAPPED_BIT;
  alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

  s_Data.light_buffer = s_Data.render_backend->create_buffer(
      sizeof(Light) * ASHFAULT_MAX_LIGHTS + sizeof(int),
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, alloc_info);
  Renderer::create_pipelines();
  Renderer::create_descriptors();
  vmaMapMemory(s_Data.render_backend->allocator(), s_Data.light_buffer.second,
               &s_Data.light_buffer_mapping);
}

void Renderer::submit_imgui_data(ImDrawData *draw_data) {
  if (draw_data) {
    ImGui_ImplVulkan_RenderDrawData(
        draw_data, s_Data.default_target->command_buffer(s_Data.current_frame));
  }
}

void Renderer::push_render_target(std::shared_ptr<RenderTarget> target) {
  s_Data.targets.push_back(target);
  VkCommandBuffer cmd = render_target().command_buffer(s_Data.current_frame);
  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd, &begin_info);

  target->begin_rendering(s_Data.image_index, s_Data.current_frame);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          s_Data.pipeline_layout, 1, 1,
                          &s_Data.texture_descriptors, 0, nullptr);
  std::memcpy(s_Data.light_buffer_mapping, s_Data.lights.data(),
              sizeof(Light) * s_Data.lights.size());
  std::memcpy(reinterpret_cast<char *>(s_Data.light_buffer_mapping) +
                  sizeof(Light) * ASHFAULT_MAX_LIGHTS,
              &s_Data.light_count, sizeof(std::int32_t));

  VkDescriptorBufferInfo buffer_info{};
  buffer_info.buffer = s_Data.light_buffer.first;
  buffer_info.offset = 0;
  buffer_info.range = sizeof(Light) * ASHFAULT_MAX_LIGHTS + sizeof(int);

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  write.dstBinding = 0;
  write.dstSet = 0;
  write.pBufferInfo = &buffer_info;
  vkCmdPushDescriptorSetKHR(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            s_Data.pipeline_layout, 0, 1, &write);
  s_Data.light_count = 0;
}

void Renderer::pop_render_target() {
  render_target().end_rendering(s_Data.image_index, s_Data.current_frame,
                                false);
  vkEndCommandBuffer(render_target().command_buffer(s_Data.current_frame));
  s_Data.cmd_to_submit.push_back(
      render_target().command_buffer(s_Data.current_frame));
  s_Data.targets.pop_back();
}

RenderTarget &Renderer::render_target() {
  return *(s_Data.targets.empty() ? s_Data.default_target
                                  : s_Data.targets.back());
}

std::shared_ptr<RenderTarget> Renderer::create_render_target(
    std::uint32_t width, std::uint32_t height, bool msaa, bool swapchain) {
  auto image_count = s_Data.swapchain->image_count();
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
          width, height,
          msaa ? s_Data.render_backend->msaa_samples() : VK_SAMPLE_COUNT_1_BIT,
          VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, alloc_info);
  VkImageView depth_view = s_Data.render_backend->create_image_view(
      depth_image.first, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);

  for (std::size_t i = 0; i < image_count; i++) {
    if (swapchain) {
      images[i] = s_Data.swapchain->image(i);
      views[i] = s_Data.swapchain->image_view(i);
    } else {
      auto image = s_Data.render_backend->create_image(
          width, height,
          msaa ? s_Data.render_backend->msaa_samples() : VK_SAMPLE_COUNT_1_BIT,
          s_Data.swapchain->surface_format().format, VK_IMAGE_TILING_OPTIMAL,
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
          alloc_info);
      images[i] = image.first;
      allocs[i] = image.second;

      auto view = s_Data.render_backend->create_image_view(
          image.first, s_Data.swapchain->surface_format().format,
          VK_IMAGE_ASPECT_COLOR_BIT);
      views[i] = view;
    }
  }

  VkRect2D render_area = {.offset = {.x = 0, .y = 0},
                          .extent = {.width = width, .height = height}};

  return std::make_shared<RenderTarget>(
      s_Data.render_backend, depth_image, depth_view, images, views,
      swapchain ? std::optional<std::vector<VmaAllocation>>()
                : std::make_optional(allocs),
      cmd_buffers, render_area);
}

void Renderer::submit_and_wait() {
  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = s_Data.cmd_to_submit.size();
  submit_info.pCommandBuffers = s_Data.cmd_to_submit.data();
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores =
      &s_Data.image_available_semaphores[s_Data.current_frame];
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores =
      &s_Data.render_finished_semaphores[s_Data.image_index];
  submit_info.pWaitDstStageMask = &s_Data.wait_stages;

  vkQueueSubmit(s_Data.render_backend->graphics_queue(), 1, &submit_info,
                s_Data.in_flight_fences[s_Data.current_frame]);

  std::vector<VkSemaphore> wait_semaphores{};
  wait_semaphores.push_back(
      s_Data.render_finished_semaphores[s_Data.image_index]);
  s_Data.swapchain->present(s_Data.render_backend->present_queue(),
                            wait_semaphores, s_Data.image_index);
  s_Data.current_frame = s_Data.current_frame % s_Data.swapchain->image_count();
}

void Renderer::submit_mesh(Mesh &mesh) {
  Material default_material{.albedo_texture_index = 0};
  submit_mesh(mesh, glm::identity<glm::mat4>(), default_material);
}

void Renderer::submit_mesh(Mesh &mesh, const glm::mat4 &transform,
                           const Material &material) {
  auto cmd = render_target().command_buffer(s_Data.current_frame);
  GraphicsPipeline *pipeline = nullptr;
  switch (mesh.type()) {
    case Mesh::Static:
      pipeline = s_Data.pipeline_manager->get_graphics_pipeline("blinn_phong");

      break;
  }

  s_Data.camera_data->albedo_texture_index = material.albedo_texture_index;
  s_Data.camera_data->normal_texture_index = material.normal_texture_index;
  auto data = s_Data.camera_data.value_or<PushConstants>(
      {.projection = glm::identity<glm::mat4>(),
       .view = glm::identity<glm::mat4>(),
       .model = glm::identity<glm::mat4>(),
       .albedo_texture_index = material.albedo_texture_index,
       .normal_texture_index = material.normal_texture_index});
  data.model = transform;
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle());
  vkCmdPushConstants(cmd, pipeline->layout(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                     sizeof(PushConstants), &data);
  VkDeviceSize offset = 0;
  vkCmdBindIndexBuffer(cmd, mesh.index_buffer()->handle(), 0,
                       index_type<std::uint32_t>::value);
  vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertex_buffer()->handle(), &offset);
  vkCmdDrawIndexed(cmd, mesh.index_buffer()->count(), 1, 0, 0, 0);
}

void Renderer::shutdown() {
  auto device = s_Data.render_backend->device();
  vkDeviceWaitIdle(device);
  vmaUnmapMemory(s_Data.render_backend->allocator(),
                 s_Data.light_buffer.second);
  vmaDestroyBuffer(s_Data.render_backend->allocator(),
                   s_Data.light_buffer.first, s_Data.light_buffer.second);
  std::for_each(s_Data.targets.begin(), s_Data.targets.end(),
                [](auto target) { target->destroy(); });
  s_Data.default_target->destroy();
  s_Data.default_target.reset();
  s_Data.targets.clear();
  s_Data.pipeline_manager->destroy();
  vkFreeDescriptorSets(device, s_Data.texture_pool, 1,
                       &s_Data.texture_descriptors);
  vkDestroyDescriptorPool(device, s_Data.texture_pool, nullptr);
  for (auto layout : s_Data.texture_layouts)
    vkDestroyDescriptorSetLayout(device, layout, nullptr);
  for (auto layout : s_Data.push_descriptor_layouts)
    vkDestroyDescriptorSetLayout(device, layout, nullptr);
  for (auto semaphore : s_Data.render_finished_semaphores)
    vkDestroySemaphore(device, semaphore, nullptr);
  for (auto semaphore : s_Data.image_available_semaphores)
    vkDestroySemaphore(device, semaphore, nullptr);
  for (auto fence : s_Data.in_flight_fences)
    vkDestroyFence(device, fence, nullptr);
  for (auto &tex : s_Data.textures) tex.destroy();
  vkDestroySampler(device, s_Data.sampler, nullptr);
  s_Data.render_backend->shutdown();
}

VulkanRenderer &Renderer::vulkan_backend() { return *s_Data.render_backend; }

std::uint32_t Renderer::swapchain_image_index() { return s_Data.image_index; }

std::shared_ptr<Mesh> Renderer::create_mesh(
    Mesh::MeshType type, const std::vector<Mesh::Vertex> &vertices,
    const std::vector<std::uint32_t> &indices) {
  auto vertex_buffer = s_Data.render_backend->create_vertex_buffer(vertices);
  auto index_buffer = s_Data.render_backend->create_index_buffer(indices);
  return std::make_shared<Mesh>(type, vertex_buffer, index_buffer);
}

void Renderer::begin_scene(Camera &camera) {
  PushConstants data = {.projection = camera.projection(),
                        .view = camera.view()};

  s_Data.camera_data = data;
}

void Renderer::end_scene() {
  s_Data.camera_data = std::optional<PushConstants>();
}

std::uint32_t Renderer::frame_index() { return s_Data.current_frame; }

void Renderer::add_light(const Light &light) {
  if (s_Data.light_count < ASHFAULT_MAX_LIGHTS) {
    s_Data.lights[s_Data.light_count++] = light;
  }
}

int Renderer::upload_texture(const char *pixels, std::uint32_t width,
                             std::uint32_t height) {
  auto texture = s_Data.render_backend->create_texture(width, height, pixels);
  std::size_t tex_index = s_Data.textures.size();
  s_Data.textures.push_back(texture);

  VkDescriptorImageInfo image_info{};
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_info.imageView = texture.image_view();
  image_info.sampler = s_Data.sampler;

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorCount = 1;
  write.dstSet = s_Data.texture_descriptors;
  write.dstBinding = 0;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.pImageInfo = &image_info;
  write.dstArrayElement = tex_index;

  vkUpdateDescriptorSets(s_Data.render_backend->device(), 1, &write, 0,
                         nullptr);
  return tex_index;
}
}  // namespace ashfault
