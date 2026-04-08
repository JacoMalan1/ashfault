#include <ashfault/core/event/scene_change.h>
#include <memory>

namespace ashfault {
SceneChangeEvent::SceneChangeEvent(std::shared_ptr<Scene> scene)
    : m_Scene(scene) {}

Event::EventType SceneChangeEvent::static_type() { return Event::SceneChange; }

Event::EventType SceneChangeEvent::event_type() const {
  return Event::SceneChange;
}

const std::shared_ptr<Scene> &SceneChangeEvent::scene() const {
  return m_Scene;
}
}  // namespace ashfault
