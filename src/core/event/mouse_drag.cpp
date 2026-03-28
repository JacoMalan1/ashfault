#include <ashfault/core/event/mouse_drag.h>

namespace ashfault {
MouseDragEvent::MouseDragEvent(MouseDragEvent::MouseButton button, float x,
                               float y)
    : Event(), m_DeltaX(x), m_DeltaY(y), m_Button(button) {}

Event::EventType MouseDragEvent::static_type() { return Event::MouseDrag; }

Event::EventType MouseDragEvent::event_type() const { return Event::MouseDrag; }

float MouseDragEvent::delta_x() const { return m_DeltaX; }

float MouseDragEvent::delta_y() const { return m_DeltaY; }

MouseDragEvent::MouseButton MouseDragEvent::button() const { return m_Button; }
}  // namespace ashfault
