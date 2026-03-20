#ifndef ASHFAULT_CAMERA_H
#define ASHFAULT_CAMERA_H

#include <glm/ext/quaternion_float.hpp>
#include <glm/glm.hpp>

namespace ashfault {
class Camera {
 public:
  Camera() = default;
  virtual ~Camera() = default;

  virtual glm::mat4 projection() = 0;
  virtual glm::mat4 view() = 0;
};

class PerspectiveCamera : public Camera {
 public:
  PerspectiveCamera(float fov, float aspect_ratio);
  ~PerspectiveCamera();

  glm::mat4 projection() override;
  glm::mat4 view() override;

  void set_fov(float fov);
  void set_aspect_ratio(float aspect_ratio);
  void set_aspect_ratio(float width, float height);

  glm::vec3 &position();
  const glm::vec3 &position() const;
  void set_position(const glm::vec3 &position);

  void pan(const glm::vec3 &delta);
  void zoom(float delta);

  glm::vec3 &rotation();
  const glm::vec3 &rotation() const;
  void set_rotation(const glm::vec3 &position);

 private:
  float m_FOV, m_AspectRatio, m_Radius;
  glm::vec3 m_Position, m_Rotation;
  glm::quat m_Orientation;
};

class OrthoCamera : public Camera {
 public:
  glm::mat4 projection() override;
  glm::mat4 view() override;

 private:
  glm::vec3 m_Position, m_Rotation;
};
}  // namespace ashfault

#endif
