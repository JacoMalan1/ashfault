#include <ashfault/core/mesh.h>
#include <ashfault/core/vertex.h>
#include <ashfault/renderer/renderer.h>
#include <spdlog/spdlog.h>

#include <ashfault/core/timer.hpp>
#include <chrono>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

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
  SPDLOG_INFO("Loading mesh `{}`", path);
  Timer<std::chrono::high_resolution_clock> timer{};
  timer.start();
  Assimp::Importer importer;
  const auto *scene = importer.ReadFile(
      path, aiProcess_CalcTangentSpace | aiProcess_GenNormals |
                aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

  if (!scene) {
    throw std::runtime_error("Failed to load model file");
  }

  const auto *mesh = scene->mMeshes[0];
  const auto *faces = mesh->mFaces;
  std::vector<Mesh::Vertex> vertices{};
  std::vector<std::uint32_t> indices{};
  vertices.reserve(mesh->mNumVertices);
  indices.reserve(mesh->mNumFaces * 3);
  auto uv_channel = mesh->mTextureCoords[0];

  for (std::size_t i = 0; i < mesh->mNumVertices; i++) {
    auto pos = mesh->mVertices[i];

    glm::vec3 position(pos.x, pos.y, pos.z);

    auto n = mesh->mNormals[i];
    glm::vec3 normal = glm::vec3(n.x, n.y, n.z);

    glm::vec2 uv(0.0f);
    if (uv_channel) {
      uv = glm::vec2(uv_channel[i].x, uv_channel[i].y);
    }

    glm::vec4 tangent(0.0f);
    if (mesh->HasTangentsAndBitangents()) {
      auto t = mesh->mTangents[i];
      auto b = mesh->mBitangents[i];

      glm::vec3 T(t.x, t.y, t.z);
      glm::vec3 B(b.x, b.y, b.z);

      float handedness =
          (glm::dot(glm::cross(normal, T), B) < 0.0f) ? -1.0f : 1.0f;

      tangent = glm::vec4(T, handedness);
    }

    vertices.push_back(Mesh::Vertex{tangent, position, normal, uv});
  }

  for (std::size_t f = 0; f < scene->mMeshes[0]->mNumFaces; f++) {
    for (std::size_t i = 0; i < faces[f].mNumIndices; i++) {
      indices.push_back(faces[f].mIndices[i]);
    }
  }

  auto vbuf = Renderer::vulkan_backend().create_vertex_buffer(vertices);
  auto ibuf = Renderer::vulkan_backend().create_index_buffer(indices);

  auto elapsed = timer.reset();
  SPDLOG_INFO("Loaded {} vertices and {} indices in {}ms", vertices.size(),
              indices.size(), elapsed);
  return std::make_shared<Mesh>(Mesh::Static, vbuf, ibuf);
}

void Mesh::destroy() {
  m_IndexBuffer->destroy();
  m_VertexBuffer->destroy();
}
}  // namespace ashfault
