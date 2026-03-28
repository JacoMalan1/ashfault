#include <ashfault/core/camera.h>
#include <iso646.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/fwd.hpp>

namespace ashfault {
PerspectiveCamera::PerspectiveCamera(float fov, float aspect_ratio)
    : Camera(),
      m_FOV(fov),
      m_AspectRatio(aspect_ratio),
      m_Radius(5.0f),
      m_Orientation(glm::quat(1, 0, 0, 0)) {}

PerspectiveCamera::~PerspectiveCamera() {}

glm::mat4 PerspectiveCamera::projection() {
  auto proj = glm::perspectiveRH_ZO(m_FOV, m_AspectRatio, 0.01f, 1000.0f);
  proj[1][1] *= -1.0f;
  return proj;
}

glm::mat4 PerspectiveCamera::view() {
  float yaw = -m_Rotation.x;
  float pitch = -m_Rotation.y;

  glm::quat yaw_quat = glm::angleAxis(yaw, glm::vec3(0, 1, 0));

  glm::vec3 right = m_Orientation * glm::vec3(1, 0, 0);
  glm::quat pitch_quat = glm::angleAxis(pitch, right);

  m_Orientation = glm::normalize(yaw_quat * pitch_quat * m_Orientation);

  {
    glm::vec3 forward = m_Orientation * glm::vec3(0, 0, -1);
    float dot = glm::dot(forward, glm::vec3(0, 1, 0));

    if (dot > 0.99f || dot < -0.99f) {
      m_Orientation = glm::normalize(yaw_quat * m_Orientation);
    }
  }

  glm::vec3 offset = m_Orientation * glm::vec3(0, 0, m_Radius);
  glm::vec3 cameraPos = m_Position + offset;
  glm::vec3 up = m_Orientation * glm::vec3(0, 1, 0);

  return glm::lookAt(cameraPos, m_Position, up);
}

void PerspectiveCamera::pan(const glm::vec3 &delta) {
  auto right = m_Orientation * glm::vec3(1, 0, 0);
  auto up = m_Orientation * glm::vec3(0, 1, 0);

  m_Position += -right * delta.x;
  m_Position += up * delta.y;
}

void PerspectiveCamera::zoom(float delta) { m_Radius += delta; }

void PerspectiveCamera::set_fov(float fov) { m_FOV = fov; }
void PerspectiveCamera::set_aspect_ratio(float aspect_ratio) {
  m_AspectRatio = aspect_ratio;
}

void PerspectiveCamera::set_aspect_ratio(float width, float height) {
  m_AspectRatio = width / height;
}

glm::vec3 &PerspectiveCamera::rotation() { return m_Rotation; }

const glm::vec3 &PerspectiveCamera::rotation() const { return m_Rotation; }

glm::vec3 &PerspectiveCamera::position() { return m_Position; }
const glm::vec3 &PerspectiveCamera::position() const { return m_Position; }

void PerspectiveCamera::set_position(const glm::vec3 &position) {
  m_Position = position;
}

void PerspectiveCamera::set_rotation(const glm::vec3 &rotation) {
  m_Rotation = rotation;
}

float PerspectiveCamera::radius() const { return m_Radius; }

void PerspectiveCamera::set_radius(float radius) { m_Radius = radius; }

void PerspectiveCamera::set_orientation(glm::quat orientation) {
  m_Orientation = orientation;
}

glm::mat4 OrthoCamera::projection() { return glm::identity<glm::mat4>(); }

glm::mat4 OrthoCamera::view() { return glm::identity<glm::mat4>(); }
}  // namespace ashfault
