#include <ashfault/core/mesh.h>
#include <ashfault/core/vertex.h>
#include <ashfault/renderer/renderer.h>
#include <spdlog/spdlog.h>
#include <tiny_obj_loader.h>

#include <ashfault/core/timer.hpp>
#include <chrono>
#include <vector>

namespace ashfault {
Mesh::Mesh(MeshType type, std::shared_ptr<VulkanBuffer> vertices,
           std::shared_ptr<VulkanBuffer> indices)
    : m_VertexBuffer(vertices), m_IndexBuffer(indices), m_Type(type) {}

Mesh::MeshType Mesh::type() const { return this->m_Type; }

std::shared_ptr<VulkanBuffer> Mesh::vertex_buffer() {
  return this->m_VertexBuffer;
}

std::shared_ptr<VulkanBuffer> Mesh::index_buffer() {
  return this->m_IndexBuffer;
}

std::shared_ptr<Mesh> Mesh::load_from_file(const std::string &path) {
  SPDLOG_DEBUG("Loading mesh '{}'", path);
  Timer<std::chrono::high_resolution_clock> timer{};
  timer.start();

  tinyobj::attrib_t attribs;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;
  if (!tinyobj::LoadObj(&attribs, &shapes, &materials, &err, path.c_str())) {
    throw std::runtime_error(err);
  }

  std::unordered_map<Mesh::Vertex, std::uint32_t> vertex_set;
  std::vector<Mesh::Vertex> vertices;
  std::vector<std::uint32_t> indices;
  vertex_set.reserve(shapes[0].mesh.indices.size());
  vertices.reserve(shapes[0].mesh.indices.size());
  indices.reserve(shapes[0].mesh.indices.size());

  for (std::size_t f = 0; f < shapes[0].mesh.indices.size(); f++) {
    tinyobj::index_t idx = shapes[0].mesh.indices[f];

    tinyobj::real_t vx = attribs.vertices[3 * idx.vertex_index + 0];
    tinyobj::real_t vy = attribs.vertices[3 * idx.vertex_index + 1];
    tinyobj::real_t vz = attribs.vertices[3 * idx.vertex_index + 2];

    tinyobj::real_t vnx = attribs.normals[3 * idx.normal_index + 0];
    tinyobj::real_t vny = attribs.normals[3 * idx.normal_index + 1];
    tinyobj::real_t vnz = attribs.normals[3 * idx.normal_index + 2];

    tinyobj::real_t uvx = attribs.texcoords[2 * idx.texcoord_index + 0];
    tinyobj::real_t uvy = attribs.texcoords[2 * idx.texcoord_index + 1];

    Mesh::Vertex vert = {
        .position = glm::vec3(vx, vy, vz),
        .normal = glm::vec3(vnx, vny, vnz),
        .uv = glm::vec2(uvx, uvy),
    };

    if (vertex_set.contains(vert)) {
      indices.push_back(vertex_set[vert]);
    } else {
      auto new_idx = vertices.size();
      vertex_set.emplace(vert, new_idx);
      indices.push_back(new_idx);
      vertices.push_back(vert);
    }
  }
  vertices.shrink_to_fit();

  SPDLOG_INFO("Loaded {} vertices, {} indices in {:.2f}ms", vertices.size(),
              indices.size(), timer.reset());
  return Renderer::create_mesh(Mesh::Static, vertices, indices);
}

void Mesh::destroy() {
  m_IndexBuffer->destroy();
  m_VertexBuffer->destroy();
}
}  // namespace ashfault
