#include <ashfault/core/event/viewport_resize.h>

#include <cstdint>

namespace ashfault {
ViewportResizeEvent::ViewportResizeEvent(std::uint32_t width,
                                         std::uint32_t height)
    : Event(), m_Width(width), m_Height(height) {}

Event::EventType ViewportResizeEvent::static_type() {
  return Event::ViewportResize;
}

Event::EventType ViewportResizeEvent::event_type() const {
  return Event::ViewportResize;
}

std::uint32_t ViewportResizeEvent::width() const { return m_Width; }

std::uint32_t ViewportResizeEvent::height() const { return m_Height; }
}  // namespace ashfault
