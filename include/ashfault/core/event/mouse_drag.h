#ifndef ASHFAULT_EVENT_MOUSE_DRAG_H
#define ASHFAULT_EVENT_MOUSE_DRAG_H

#include <ashfault/ashfault.h>
#include <ashfault/core/event.h>

namespace ashfault {
class ASHFAULT_API MouseDragEvent : public Event {
 public:
  enum MouseButton { Left, Right, Middle };

  MouseDragEvent(MouseButton button, float x, float y);
  ~MouseDragEvent() = default;

  static Event::EventType static_type();

  EventType event_type() const override;

  MouseButton button() const;

  float delta_x() const;
  float delta_y() const;

 private:
  float m_DeltaX, m_DeltaY;
  MouseButton m_Button;
};
}  // namespace ashfault

#endif
