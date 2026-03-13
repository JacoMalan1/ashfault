#include <ashfault/core/event/key_press.h>

namespace ashfault {
KeyPressEvent::KeyPressEvent(int key_code) : m_KeyCode(key_code) {}

Event::EventType KeyPressEvent::static_type() {
  return Event::KeyPress;
}

Event::EventType KeyPressEvent::event_type() const {
  return Event::KeyPress;
}

int KeyPressEvent::key_code() const {
  return m_KeyCode;
}
}
