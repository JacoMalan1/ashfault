#include <ashfault/editor/event/state_change.h>

namespace ashfault {
StateChangeEvent::StateChangeEvent(State::RuntimeState state)
    : Event(), m_State(state) {}
StateChangeEvent::~StateChangeEvent() {}

Event::EventType StateChangeEvent::static_type() { return Event::StateChange; }

Event::EventType StateChangeEvent::event_type() const {
  return Event::StateChange;
}

State::RuntimeState StateChangeEvent::state() const { return m_State; }
}  // namespace ashfault
