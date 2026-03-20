#ifndef ASHFAULT_MESH_H
#define ASHFAULT_MESH_H

#include <ashfault/ashfault.h>
#include <ashfault/renderer/vkrenderer.h>

#include <ashfault/renderer/buffer.hpp>
#include <glm/glm.hpp>
#include <memory>

namespace ashfault {
class ASHFAULT_API Mesh {
 public:
  enum MeshType { Static };
  struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
  };

  Mesh(MeshType type, std::shared_ptr<VulkanBuffer> vertex_buffer,
       std::shared_ptr<VulkanBuffer> index_buffer);

  std::shared_ptr<VulkanBuffer> vertex_buffer();
  std::shared_ptr<VulkanBuffer> index_buffer();
  MeshType type() const;

  static std::shared_ptr<Mesh> load_from_file(const std::string &path);

 private:
  std::shared_ptr<VulkanBuffer> m_VertexBuffer, m_IndexBuffer;
  MeshType m_Type;
};
}  // namespace ashfault

#endif
