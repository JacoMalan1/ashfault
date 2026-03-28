#ifndef ASHFAULT_EVENT_BUS_H
#define ASHFAULT_EVENT_BUS_H

#include <ashfault/ashfault.h>
#include <ashfault/core/event.h>
#include <algorithm>
#include <type_traits>
#include <vector>
#include <functional>

namespace ashfault {
template <class T>
  requires std::is_base_of<Event, T>::value
class ASHFAULT_API EventBus {
public:
  static EventBus &get() {
    static EventBus instance{};
    return instance;
  }

  void dispatch(const T &event) {
    for (auto &sub : m_Subscribers) {
      std::invoke(sub, event);
    }
  }

  void subscribe(std::function<void(const T &)> callback) {
    m_Subscribers.push_back(callback);
  }

  void unsubscribe(std::function<void(const T &)> callback) {
    auto it = std::find(m_Subscribers.begin(), m_Subscribers.end(), callback);
    if (it != m_Subscribers.end()) {
      m_Subscribers.erase(it);
    }
  }

private:
  std::vector<std::function<void(T)>> m_Subscribers;

  EventBus() = default;
};
}  // namespace ashfault

#endif
