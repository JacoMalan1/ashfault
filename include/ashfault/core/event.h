#ifndef ASHFAULT_EVENT_H
#define ASHFAULT_EVENT_H

#include <ashfault/ashfault.h>
#include <functional>
#include <type_traits>

namespace ashfault {
class ASHFAULT_API Event {
public:
  Event();
  virtual ~Event() = default;

  enum EventType {
    KeyPress
  };

  virtual EventType event_type() const = 0;

  bool is_handled() const;
  void set_handled();

private:
  bool m_Handled;
};

class Dispatcher {
public:
  template<typename T>
    requires std::is_base_of<Event, T>::value
  void dispatch(Event &event, std::function<void(T &)> f) {
    if (event.event_type() == T::static_type()) {
      f(static_cast<T&>(event));
    }
  }
};
}

#endif
