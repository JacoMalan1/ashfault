#ifndef ASHFAULT_CAMERA_H
#define ASHFAULT_CAMERA_H

#include <glm/glm.hpp>
#include <memory>
#include <optional>

namespace ashfault {
class EditorCamera {
public:
  EditorCamera(const glm::vec3 &position, const glm::vec3 &rotation);

  EditorCamera(const EditorCamera &) = default;
  virtual EditorCamera &operator=(const EditorCamera &) = default;
  virtual ~EditorCamera() = default;

  virtual glm::mat4 projection() const = 0;
  virtual glm::mat4 view() const = 0;

  void rotate(const glm::vec3 &rotation);
  void move(const glm::vec3 &delta);

  void set_rotation(const glm::vec3 &rotation);
  void set_position(const glm::vec3 &position);

protected:
  glm::vec3 m_Position, m_Rotation;

  glm::mat4 m_Projection;
  glm::mat4 m_View;
};

class PerspectiveEditorCameraBuilder;
class PerspectiveEditorCamera : public EditorCamera {
public:
  friend class PerspectiveCameraControls;
  PerspectiveEditorCamera(const glm::vec3 &position, const glm::vec3 &rotation,
                          float fov, float aspect_ratio);
  PerspectiveEditorCamera(const PerspectiveEditorCamera &) = default;
  PerspectiveEditorCamera &operator=(const PerspectiveEditorCamera &) = default;
  ~PerspectiveEditorCamera() = default;

  static PerspectiveEditorCameraBuilder builder();

  float fov();

  glm::mat4 projection() const override;
  glm::mat4 view() const override;

private:
  float m_Fov, m_AspectRatio;
};

class PerspectiveEditorCameraBuilder {
public:
  PerspectiveEditorCameraBuilder() = default;

  PerspectiveEditorCameraBuilder &position(const glm::vec3 &initial_position);
  PerspectiveEditorCameraBuilder &rotation(const glm::vec3 &initial_rotation);
  PerspectiveEditorCameraBuilder &fov(float fov);
  PerspectiveEditorCameraBuilder &aspect_ratio(float aspect);

  PerspectiveEditorCamera build();

private:
  glm::vec3 m_Position = glm::vec3(0.0f), m_Rotation = glm::vec3(0.0f);
  float m_Fov = 90.0f;
  std::optional<float> m_AspectRatio;
};

class OrthoEditorCamera;
class OrthoEditorCameraBuilder {
public:
  OrthoEditorCameraBuilder() = default;

  OrthoEditorCameraBuilder &position(const glm::vec3 &initial_position);
  OrthoEditorCameraBuilder &rotation(const glm::vec3 &initial_rotation);
  OrthoEditorCameraBuilder &left(float left);
  OrthoEditorCameraBuilder &right(float right);
  OrthoEditorCameraBuilder &top(float top);
  OrthoEditorCameraBuilder &bottom(float bottom);
  OrthoEditorCameraBuilder &near(float near);
  OrthoEditorCameraBuilder &far(float far);

  OrthoEditorCamera &build();

private:
  glm::vec3 m_Position, m_Rotation;
  float m_Left, m_Right, m_Top, m_Bottom, m_Near, m_Far;
};

class OrthoEditorCamera : public EditorCamera {
public:
  OrthoEditorCamera(const glm::vec3 &position, const glm::vec3 &rotation,
                    float left, float right, float top, float bottom,
                    float z_near, float z_far);
  OrthoEditorCamera(const OrthoEditorCamera &) = default;
  OrthoEditorCamera &operator=(const OrthoEditorCamera &) = default;
  ~OrthoEditorCamera() = default;

  static OrthoEditorCameraBuilder builder();

  glm::mat4 projection() const override;
  glm::mat4 view() const override;
};

class EditorCameraControls {
public:
  virtual void render_controls() = 0;
  virtual void resize(float width, float height) = 0;
};

class PerspectiveCameraControls : public EditorCameraControls {
public:
  PerspectiveCameraControls(std::shared_ptr<PerspectiveEditorCamera> camera);

  PerspectiveCameraControls(const PerspectiveCameraControls &) = delete;
  PerspectiveCameraControls &
  operator=(const PerspectiveCameraControls &) = delete;
  ~PerspectiveCameraControls() = default;

  void render_controls() override;
  void resize(float width, float height) override;

private:
  std::shared_ptr<PerspectiveEditorCamera> m_Camera;
};

class OrthoCameraControls : public EditorCameraControls {
public:
  OrthoCameraControls(const OrthoCameraControls &) = delete;
  OrthoCameraControls &operator=(const OrthoCameraControls &) = delete;
  ~OrthoCameraControls() = default;

  void render_controls() override;

private:
  std::shared_ptr<OrthoEditorCamera> m_Camera;
};
}; // namespace ashfault

#endif
