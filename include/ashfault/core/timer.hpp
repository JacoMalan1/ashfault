#ifndef ASHFAULT_TIMER_H
#define ASHFAULT_TIMER_H

#include <ashfault/ashfault.h>
#include <chrono>
#include <functional>

namespace ashfault {
template <class Clock = std::chrono::system_clock>
  requires std::chrono::is_clock<Clock>::value
class ASHFAULT_API Timer {
public:
  Timer() {}

  void start() { m_StartTime = Clock::now(); }

  float reset() {
    auto now = Clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
            now - m_StartTime);
    m_StartTime = now;
    return elapsed.count();
  }

  static float time(std::function<void()> op) {
    auto t = Timer();
    t.start();
    op();
    return t.reset();
  }

private:
  std::chrono::time_point<Clock, typename Clock::duration> m_StartTime;
};
} // namespace ashfault

#endif
