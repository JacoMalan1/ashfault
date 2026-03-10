#ifndef ASHFAULT_MESH_H
#define ASHFAULT_MESH_H

#include "ashfault/renderer/renderer.h"
#include <CLSTL/shared_ptr.h>
#include <ashfault/renderer/buffer.hpp>

namespace ashfault {
class Mesh {
public:
  enum MeshType { Static };

  Mesh(MeshType type, clstl::shared_ptr<VulkanBuffer> vertex_buffer,
       clstl::shared_ptr<VulkanBuffer> index_buffer);

  clstl::shared_ptr<VulkanBuffer> vertex_buffer();
  clstl::shared_ptr<VulkanBuffer> index_buffer();
  MeshType type() const;

  static clstl::shared_ptr<Mesh> load_from_file(const std::string &path, Renderer *);

private:
  clstl::shared_ptr<VulkanBuffer> m_VertexBuffer, m_IndexBuffer;
  MeshType m_Type;
};
} // namespace ashfault

#endif
