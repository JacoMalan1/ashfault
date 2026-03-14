#include <ashfault/core/event/key_press.h>

namespace ashfault {
KeyPressEvent::KeyPressEvent(int key_code, int action) : m_KeyCode(key_code), m_Action(action) {}

Event::EventType KeyPressEvent::static_type() {
  return Event::KeyPress;
}

Event::EventType KeyPressEvent::event_type() const {
  return Event::KeyPress;
}

int KeyPressEvent::key_code() const {
  return m_KeyCode;
}

int KeyPressEvent::action() const {
  return m_Action;
}
}
