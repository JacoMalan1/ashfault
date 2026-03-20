#ifndef ASHFAULT_EVENT_MOUSE_SCROLL_H
#define ASHFAULT_EVENT_MOUSE_SCROLL_H

#include <ashfault/core/event.h>

namespace ashfault {
class MouseScrollEvent : public Event {
 public:
  MouseScrollEvent(float delta);
  ~MouseScrollEvent() = default;

  static Event::EventType static_type();

  EventType event_type() const override;

  float delta() const;

 private:
  float m_Delta;
};
}  // namespace ashfault

#endif
