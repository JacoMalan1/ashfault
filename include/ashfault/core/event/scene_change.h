#ifndef ASHFAULT_EVENT_SCENE_CHANGE_H
#define ASHFAULT_EVENT_SCENE_CHANGE_H

#include <memory>
#include <ashfault/core/event.h>
#include <ashfault/core/scene.h>

namespace ashfault {
class SceneChangeEvent : public Event {
public:
  SceneChangeEvent(std::shared_ptr<Scene> scene);
  ~SceneChangeEvent() = default;

  static Event::EventType static_type();

  Event::EventType event_type() const override;

  const std::shared_ptr<Scene> &scene() const;

private:
  std::shared_ptr<Scene> m_Scene;
};
}  // namespace ashfault

#endif
