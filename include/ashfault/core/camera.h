#ifndef ASHFAULT_CAMERA_H
#define ASHFAULT_CAMERA_H

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

private:
  float m_FOV, m_AspectRatio; 
  glm::vec3 m_Position, m_Rotation;
};

class OrthoCamera : public Camera {
public:
  glm::mat4 projection() override;
  glm::mat4 view() override;

private:
  glm::vec3 m_Position, m_Rotation;
};
}

#endif
