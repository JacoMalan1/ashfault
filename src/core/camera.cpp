#include <ashfault/core/camera.h>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"

namespace ashfault {
PerspectiveCamera::PerspectiveCamera(float fov, float aspect_ratio)
    : Camera(), m_FOV(fov), m_AspectRatio(aspect_ratio) {}

PerspectiveCamera::~PerspectiveCamera() {}

glm::mat4 PerspectiveCamera::projection() {
  return glm::perspectiveLH_NO(m_FOV, m_AspectRatio, 0.01f, 1000.0f);
}

glm::mat4 PerspectiveCamera::view() { return glm::identity<glm::mat4>(); }
}  // namespace ashfault
