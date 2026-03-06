#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/constants.hpp>
#include <imgui.h>
#define TINYOBJLOADER_IMPLEMENTATION

#include "ashfault/descriptor_set.h"
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
#include <tiny_obj_loader.h>

void error_callback(int error, const char *desc) {
  spdlog::error("GLFW ({}): {}", error, desc);
}

struct UBO {
  glm::mat4 proj_mat;
  glm::mat4 view_mat;
  glm::mat4 model_mat;
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
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
        glfwCreateWindow(1280, 720, "Hello World!", nullptr, nullptr);
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

    VkVertexInputAttributeDescription norm{};
    norm.binding = 0;
    norm.location = 1;
    norm.offset = 3 * sizeof(float);
    norm.format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions.push_back(pos);
    descriptions.push_back(norm);

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    auto ret =
        tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "monkey.obj");
    if (!ret) {
      throw std::runtime_error("Failed to load model");
    }

    clstl::vector<Vertex> vertices;

    for (std::size_t f = 0; f < shapes[0].mesh.indices.size(); f++) {
      tinyobj::index_t idx = shapes[0].mesh.indices[f];

      tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
      tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
      tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

      tinyobj::real_t vnx = attrib.normals[3 * idx.normal_index + 0];
      tinyobj::real_t vny = attrib.normals[3 * idx.normal_index + 1];
      tinyobj::real_t vnz = attrib.normals[3 * idx.normal_index + 2];

      Vertex vert = {glm::vec3(vx, vy, vz), glm::vec3(vnx, vny, vnz)};
      vertices.push_back(vert);
    }

    spdlog::info("Loaded {} vertices", vertices.size());
    spdlog::info("vertices[0]: x: {}, y: {}, z: {}", vertices[0].position.x,
                 vertices[0].position.y, vertices[0].position.z);

    glm::mat4 mod = glm::scale(glm::identity<glm::mat4>(), glm::vec3(20.0f));
    glm::mat4 view =
        glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 3.0f));
    view = glm::rotate(view, glm::pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f));
    UBO proj{};
    proj.proj_mat = glm::perspectiveFovLH(glm::half_pi<float>(), 1280.0f, 720.0f, 0.001f, 100.0f);
    proj.model_mat = mod;
    proj.view_mat = view;
    auto ubo_buffer = renderer->create_uniform_buffer(proj);
    {
      auto [dsets, pool] = renderer->create_descriptor_sets()
                               .add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                            VK_SHADER_STAGE_VERTEX_BIT, 1, 0)
                               .build();

      dsets[0]->update_uniform_buffer(ubo_buffer, 0);
      auto pipeline =
          renderer->create_graphics_pipeline()
              .vertex_shader(vshader)
              .fragment_shader(fshader)
              .input_attribute_descriptions(descriptions, sizeof(Vertex))
              .descriptor_sets(dsets)
              .build();

      auto vertex_buffer = renderer->create_vertex_buffer(vertices);
      spdlog::trace("Vertex count: {}", vertex_buffer->count());

      float angle = 0.0f;
      spdlog::info("Start first frame");
      while (glfwWindowShouldClose(window) != GLFW_TRUE &&
             glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        auto frame = renderer->start_frame();
        if (frame.has_value()) {
          frame.value().bind_pipeline(pipeline.get());
          frame.value().bind_descriptor_set(dsets[0].get(), pipeline.get());
          frame.value().draw(vertex_buffer.get());
	  frame.value().draw_ui([]() {
	    ImGui::Begin("Statistics");
	    ImGui::Text("Frame Time: %d", 0);
	    ImGui::End();
	  });
          frame.value().submit();
        }

	auto viewport_size = renderer->viewport_size();
        angle += 0.007f;
        mod = glm::rotate(glm::identity<glm::mat4>(), angle,
                          glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(glm::identity<glm::mat4>(),
                              glm::vec3(0.0f, 0.0f, 3.0f));
        view = glm::rotate(view, glm::pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f));

        vkDeviceWaitIdle(renderer->device());
        proj.proj_mat = glm::perspectiveFovLH(glm::half_pi<float>(), static_cast<float>(viewport_size[0]),
                                              static_cast<float>(viewport_size[1]), 0.0001f, 100.0f);
        proj.view_mat = view;
        proj.model_mat = mod;
	auto new_buf = renderer->create_uniform_buffer(proj);
	dsets[0]->update_uniform_buffer(new_buf, 0);
	ubo_buffer = new_buf;

        glfwPollEvents();
      }
    }
  }

  glfwTerminate();
}
