#include "ashfault/descriptor_set.h"
#include "glm/ext/matrix_transform.hpp"
#include <CLSTL/unique_ptr.h>
#include <ashfault/pipeline.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <ashfault/pipeline.h>
#include <ashfault/renderer.h>
#include <glm/glm.hpp>
#include <stdexcept>

void error_callback(int error, const char *desc) {
  spdlog::error("GLFW ({}): {}", error, desc);
}

struct UBO {
  glm::mat4 proj_mat;
};

struct Vertex {
  glm::vec3 position;
};

int main() {
  {
    spdlog::set_level(spdlog::level::trace);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
      throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window =
        glfwCreateWindow(800, 600, "Hello World!", nullptr, nullptr);
    if (!window) {
      throw std::runtime_error("Failed to create GLFW window");
    }

    auto renderer = clstl::make_unique<ashfault::Renderer>();

    renderer->init(window);

    auto vshader = renderer->create_shader("shader.vert.spv");
    auto fshader = renderer->create_shader("shader.frag.spv");

    clstl::vector<VkVertexInputAttributeDescription> descriptions;
    VkVertexInputAttributeDescription pos{};
    pos.binding = 0;
    pos.location = 0;
    pos.offset = 0;
    pos.format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions.push_back(pos);

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    auto [buffer, alloc] = renderer->create_buffer(
        sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, alloc_info);

    {
      auto [dsets, pool] = renderer->create_descriptor_sets()
                               .add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                            VK_SHADER_STAGE_VERTEX_BIT, 1, 0)
                               .build();
      spdlog::trace("Dsets strong count: {}", dsets[0].use_count());

      UBO *mapping;
      vmaMapMemory(renderer->allocator(), alloc,
                   reinterpret_cast<void **>(&mapping));
      UBO proj{};
      proj.proj_mat = glm::identity<glm::mat4>();
      *mapping = proj;
      vmaUnmapMemory(renderer->allocator(), alloc);

      VkDescriptorBufferInfo buffer_info{};
      buffer_info.offset = 0;
      buffer_info.buffer = buffer;
      buffer_info.range = sizeof(UBO);

      VkWriteDescriptorSet write{};
      write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write.descriptorCount = 1;
      write.dstBinding = 0;
      write.pBufferInfo = &buffer_info;
      write.dstSet = dsets[0]->handle();
      write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

      vkUpdateDescriptorSets(renderer->device(), 1, &write, 0, nullptr);
      auto pipeline =
          renderer->create_graphics_pipeline()
              .vertex_shader(vshader)
              .fragment_shader(fshader)
              .input_attribute_descriptions(descriptions, sizeof(Vertex))
              .descriptor_sets(dsets)
              .build();

      clstl::vector<Vertex> vertices;
      vertices.reserve(3);
      vertices.push_back({glm::vec3(0.0f, -0.5f, 0.0f)});
      vertices.push_back({glm::vec3(0.5, 0.5f, 0.0f)});
      vertices.push_back({glm::vec3(-0.5f, 0.5f, 0.0f)});

      clstl::vector<std::uint16_t> indices;
      indices.reserve(3);
      indices.push_back(0);
      indices.push_back(1);
      indices.push_back(2);
      auto vertex_buffer = renderer->create_vertex_buffer(vertices);
      auto index_buffer = renderer->create_index_buffer(indices);

      while (glfwWindowShouldClose(window) != GLFW_TRUE &&
             glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        auto frame = renderer->start_frame();
        if (frame.has_value()) {
          frame.value().bind_pipeline(pipeline.get());
          frame.value().bind_descriptor_set(dsets[0].get(), pipeline.get());
          frame.value().draw(vertex_buffer.get(), index_buffer.get());
          frame.value().submit();
        }

        glfwPollEvents();
      }
    }
    vmaDestroyBuffer(renderer->allocator(), buffer, alloc);
  }

  glfwTerminate();
}
