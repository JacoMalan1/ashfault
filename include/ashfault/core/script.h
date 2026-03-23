#ifndef ASHFAULT_SCRIPT_H
#define ASHFAULT_SCRIPT_H

#include <ashfault/ashfault.h>
#include <ashfault/core/entity.h>
#include <optional>
#include <string>
#include <sol/sol.hpp>

namespace ashfault {
class ASHFAULT_API Script {
public:
  Script(const std::string &source);
  ~Script();

  void init(sol::state &lua);

  void on_update(float dt, std::optional<Entity> entity);
  bool is_initialized() const;

private:
  bool m_Initialized;
  std::string m_Source;
  sol::environment m_Environment;
  sol::function m_OnUpdate;
};
}  // namespace ashfault

#endif
