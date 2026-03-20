#include <ashfault/renderer/renderer.h>
#include <spdlog/spdlog.h>
#include <ashfault/core/mesh.h>
#include <ashfault/core/vertex.h>
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

std::shared_ptr<Mesh> Mesh::load_from_file(const std::string &path) {
  tinyobj::attrib_t attribs;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;
  if (!tinyobj::LoadObj(&attribs, &shapes, &materials, &err, path.c_str())) {
    throw std::runtime_error(err);
  }

  std::vector<Mesh::Vertex> vertices;
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

    Mesh::Vertex vert = {.position = glm::vec3(vx, vy, vz),
                         .normal = glm::vec3(vnx, vny, vnz)};
    vertices.push_back(vert);
  }

  SPDLOG_INFO("Loaded {} vertices", vertices.size());
  SPDLOG_INFO("Loaded {} indices", indices.size());
  return Renderer::create_mesh(Mesh::Static, vertices, indices);
}
} // namespace ashfault
