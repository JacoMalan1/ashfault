#include <ashfault/core/event/scene_start.h>

namespace ashfault {
SceneStartEvent::SceneStartEvent(Scene *scene) : m_Scene(scene) {}

Event::EventType SceneStartEvent::static_type() { return Event::SceneStart; }

Event::EventType SceneStartEvent::event_type() const {
  return Event::SceneStart;
}

Scene *SceneStartEvent::scene() { return m_Scene; }
}  // namespace ashfault
