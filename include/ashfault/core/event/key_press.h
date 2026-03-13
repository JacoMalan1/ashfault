#ifndef ASHFAULT_KEY_PRESS_EVENT_H
#define ASHFAULT_KEY_PRESS_EVENT_H

#include <ashfault/core/event.h>

namespace ashfault {
class KeyPressEvent : public Event {
public:
  KeyPressEvent(int key_code);
  ~KeyPressEvent() = default;

  static Event::EventType static_type();

  EventType event_type() const override;

  int key_code() const;

private:
  int m_KeyCode;
};
}

#endif
