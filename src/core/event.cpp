#include <ashfault/core/event.h>

namespace ashfault {
Event::Event() : m_Handled(false) {}

bool Event::is_handled() const { return m_Handled; }

void Event::set_handled() { m_Handled = true; }
}  // namespace ashfault
