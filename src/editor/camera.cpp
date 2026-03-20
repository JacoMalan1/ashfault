#include <ashfault/editor/camera.h>
#include <imgui.h>

#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <stdexcept>

namespace ashfault {
EditorCamera::EditorCamera(const glm::vec3 &position, const glm::vec3 &rotation)
    : m_Position(position), m_Rotation(rotation) {}

PerspectiveEditorCamera::PerspectiveEditorCamera(const glm::vec3 &position,
                                                 const glm::vec3 &rotation,
                                                 float fov, float aspect_ratio)
    : EditorCamera(position, rotation),
      m_Fov(fov),
      m_AspectRatio(aspect_ratio) {}

void EditorCamera::rotate(const glm::vec3 &rotation) {
  this->m_Rotation += rotation;
}

void EditorCamera::move(const glm::vec3 &delta) {
  glm::vec3 forward =
      glm::vec3(std::cos(this->m_Rotation.x) * std::sin(this->m_Rotation.y),
                -std::sin(this->m_Rotation.x),
                std::cos(this->m_Rotation.x) * std::cos(this->m_Rotation.y));

  glm::vec3 right =
      glm::vec3(std::cos(this->m_Rotation.y), 0, -std::sin(this->m_Rotation.y));

  glm::vec3 up = glm::cross(forward, right);

  glm::vec3 movement = right * delta.x + up * delta.y + forward * delta.z;
  movement.z *= -1;
  this->m_Position += movement;
}

glm::mat4 PerspectiveEditorCamera::projection() const {
  return glm::perspectiveLH_ZO(m_Fov, m_AspectRatio, 0.1f, 1000.0f);
}

glm::mat4 PerspectiveEditorCamera::view() const {
  glm::mat4 T = glm::translate(glm::identity<glm::mat4>(), -this->m_Position);
  glm::mat4 R = glm::rotate(glm::identity<glm::mat4>(), this->m_Rotation.x,
                            glm::vec3(1.0f, 0.0f, 0.0f));
  R = glm::rotate(R, this->m_Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
  return R * T;
}

PerspectiveEditorCameraBuilder PerspectiveEditorCamera::builder() { return {}; }

PerspectiveEditorCameraBuilder &PerspectiveEditorCameraBuilder::position(
    const glm::vec3 &position) {
  this->m_Position = position;
  return *this;
}

PerspectiveEditorCameraBuilder &PerspectiveEditorCameraBuilder::rotation(
    const glm::vec3 &rotation) {
  this->m_Rotation = rotation;
  return *this;
}

PerspectiveEditorCameraBuilder &PerspectiveEditorCameraBuilder::fov(float fov) {
  this->m_Fov = fov;
  return *this;
}

PerspectiveEditorCameraBuilder &PerspectiveEditorCameraBuilder::aspect_ratio(
    float aspect) {
  this->m_AspectRatio = aspect;
  return *this;
}

PerspectiveEditorCamera PerspectiveEditorCameraBuilder::build() {
  if (!m_AspectRatio.has_value()) {
    throw std::runtime_error(
        "Cannot build perspective camera without aspect ratio");
  }

  return PerspectiveEditorCamera(m_Position, m_Rotation, m_Fov,
                                 m_AspectRatio.value());
}

PerspectiveCameraControls::PerspectiveCameraControls(
    std::shared_ptr<PerspectiveEditorCamera> camera)
    : m_Camera(camera) {}

void PerspectiveCameraControls::render_controls() {
  float fov = glm::degrees(this->m_Camera->m_Fov);
  ImGui::SliderFloat("FOV", &fov, 80.0f, 120.0f);
  this->m_Camera->m_Fov = glm::radians(fov);
}

float PerspectiveEditorCamera::fov() { return this->m_Fov; }

void PerspectiveCameraControls::resize(float width, float height) {
  this->m_Camera->m_AspectRatio = width / height;
}

void EditorCamera::set_rotation(const glm::vec3 &rotation) {
  this->m_Rotation = rotation;
}

void EditorCamera::set_position(const glm::vec3 &position) {
  this->m_Position = position;
}

OrthoEditorCamera::OrthoEditorCamera(const glm::vec3 &position,
                                     const glm::vec3 &rotation, float left,
                                     float right, float top, float bottom,
                                     float z_near, float z_far)
    : EditorCamera(position, rotation),
      m_Left(left),
      m_Right(right),
      m_Top(top),
      m_Bottom(bottom),
      m_ZNear(z_near),
      m_ZFar(z_far) {}

OrthoEditorCameraBuilder OrthoEditorCamera::builder() {
  return OrthoEditorCameraBuilder();
}

glm::mat4 OrthoEditorCamera::projection() const {
  return glm::identity<glm::mat4>();
}
glm::mat4 OrthoEditorCamera::view() const { return glm::identity<glm::mat4>(); }

OrthoCameraControls::OrthoCameraControls(
    std::shared_ptr<OrthoEditorCamera> camera)
    : m_Camera(camera) {}

void OrthoCameraControls::render_controls() {}
}  // namespace ashfault
