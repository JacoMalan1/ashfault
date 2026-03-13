#include <ashfault/core/vertex.h>
#include "ashfault/renderer/vkrenderer.h"
#include "spdlog/spdlog.h"
#include <ashfault/core/mesh.h>
#include <tiny_obj_loader.h>
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

std::shared_ptr<Mesh> Mesh::load_from_file(const std::string &path,
                                             VulkanRenderer *renderer) {
  tinyobj::attrib_t attribs;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;
  if (!tinyobj::LoadObj(&attribs, &shapes, &materials, &err, path.c_str())) {
    throw std::runtime_error(err);
  }

  std::vector<StaticVertex> vertices;
  std::vector<std::uint32_t> indices;

  for (std::size_t f = 0; f < shapes[0].mesh.indices.size(); f++) {
    indices.push_back(f);
    tinyobj::index_t idx = shapes[0].mesh.indices[f];

    tinyobj::real_t vx = attribs.vertices[3 * idx.vertex_index + 0];
    tinyobj::real_t vy = attribs.vertices[3 * idx.vertex_index + 1];
    tinyobj::real_t vz = attribs.vertices[3 * idx.vertex_index + 2];

    tinyobj::real_t vnx = attribs.normals[3 * idx.normal_index + 0];
    tinyobj::real_t vny = attribs.normals[3 * idx.normal_index + 1];
    tinyobj::real_t vnz = attribs.normals[3 * idx.normal_index + 2];

    StaticVertex vert = {glm::vec3(vx, vy, vz), glm::vec3(vnx, vny, vnz)};
    vertices.push_back(vert);
  }

  auto vertex_buffer = renderer->create_vertex_buffer(vertices);
  auto index_buffer = renderer->create_index_buffer(indices);
  SPDLOG_INFO("Loaded {} vertices", vertices.size());
  SPDLOG_INFO("Loaded {} indices", indices.size());
  return std::make_shared<Mesh>(Mesh::Static, vertex_buffer, index_buffer);
}
} // namespace ashfault
