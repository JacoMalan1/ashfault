#ifndef ASHFAULT_EDITOR_EVENT_STATE_CHANGE_H
#define ASHFAULT_EDITOR_EVENT_STATE_CHANGE_H

#include <ashfault/ashfault.h>
#include <ashfault/core/event.h>
#include <ashfault/editor/state.h>

namespace ashfault {
class ASHFAULT_API StateChangeEvent : public Event {
public:
  StateChangeEvent(State::RuntimeState new_state);
  ~StateChangeEvent();

  static Event::EventType static_type();

  Event::EventType event_type() const override;

  State::RuntimeState state() const;

private:
  State::RuntimeState m_State;
};
}  // namespace ashfault

#endif
