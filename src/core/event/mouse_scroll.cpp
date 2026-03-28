#include <ashfault/core/event/mouse_scroll.h>

namespace ashfault {
MouseScrollEvent::MouseScrollEvent(float delta) : m_Delta(delta) {}

Event::EventType MouseScrollEvent::static_type() { return Event::MouseScroll; }

Event::EventType MouseScrollEvent::event_type() const {
  return Event::MouseScroll;
}

float MouseScrollEvent::delta() const { return m_Delta; }
}  // namespace ashfault
