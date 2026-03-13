#ifndef ASHFAULT_KEY_PRESS_EVENT_H
#define ASHFAULT_KEY_PRESS_EVENT_H

#include <ashfault/core/event.h>
#include <ashfault/ashfault.h>

namespace ashfault {
class ASHFAULT_API KeyPressEvent : public Event {
public:
  KeyPressEvent(int key_code);
  ~KeyPressEvent() = default;

  static Event::EventType static_type();

  EventType event_type() const override;

  int key_code() const;

private:
  int m_KeyCode;
};
} // namespace ashfault

#endif
