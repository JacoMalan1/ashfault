#ifndef ASHFAULT_EVENT_VIEWPORT_RESIZE_H
#define ASHFAULT_EVENT_VIEWPORT_RESIZE_H

#include <ashfault/ashfault.h>
#include <ashfault/core/event.h>

#include <cstdint>

namespace ashfault {
class ASHFAULT_API ViewportResizeEvent : public Event {
public:
  ViewportResizeEvent(std::uint32_t width, std::uint32_t height);
  ~ViewportResizeEvent() = default;

  static Event::EventType static_type();

  EventType event_type() const override;

  std::uint32_t width() const;
  std::uint32_t height() const;

private:
  std::uint32_t m_Width, m_Height;
};
}  // namespace ashfault

#endif
