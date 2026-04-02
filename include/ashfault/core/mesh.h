#ifndef ASHFAULT_MESH_H
#define ASHFAULT_MESH_H
#include <ashfault/core/asset_manager.hpp>
#include <functional>
#define GLM_ENABLE_EXPERIMENTAL

#include <ashfault/ashfault.h>
#include <ashfault/renderer/vkrenderer.h>

#include <ashfault/renderer/buffer.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <glm/gtx/hash.hpp>

namespace ashfault {
class ASHFAULT_API Mesh : IAsset {
public:
  enum MeshType { Static };
  struct Vertex {
    glm::vec4 tangent;
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    bool operator==(const Vertex &other) const {
      return other.position == position && other.normal == normal &&
             other.uv == uv;
    }
  };

  Mesh(MeshType type, std::shared_ptr<VulkanBuffer> vertex_buffer,
       std::shared_ptr<VulkanBuffer> index_buffer);

  std::shared_ptr<VulkanBuffer> vertex_buffer();
  std::shared_ptr<VulkanBuffer> index_buffer();
  MeshType type() const;

  static std::shared_ptr<Mesh> load_from_file(const std::string &path);

  void destroy() override;

private:
  std::shared_ptr<VulkanBuffer> m_VertexBuffer, m_IndexBuffer;
  MeshType m_Type;
};
}  // namespace ashfault

template <>
struct std::hash<ashfault::Mesh::Vertex> {
  std::size_t operator()(const ashfault::Mesh::Vertex &vert) const noexcept {
    std::hash<glm::vec3> hasher{};
    std::hash<glm::vec2> vec2_hasher{};
    return ((hasher(vert.position) ^ (hasher(vert.normal) << 1)) >> 1) ^
           (vec2_hasher(vert.uv) << 1);
  }
};

#endif
