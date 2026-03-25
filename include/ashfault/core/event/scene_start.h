#ifndef ASHFAULT_EVENT_SCENE_START_H
#define ASHFAULT_EVENT_SCENE_START_H

#include <ashfault/ashfault.h>
#include <ashfault/core/event.h>
#include <ashfault/core/scene.h>

namespace ashfault {
class ASHFAULT_API SceneStartEvent : public Event {
public:
  SceneStartEvent(Scene *scene);
  ~SceneStartEvent() = default;

  static Event::EventType static_type();

  Event::EventType event_type() const override;

  Scene *scene();

private:
  Scene *m_Scene;
};
}  // namespace ashfault

#endif
