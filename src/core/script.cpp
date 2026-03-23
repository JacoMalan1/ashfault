#include <ashfault/core/script.h>
#include <spdlog/spdlog.h>

#include <optional>
#include <sol/forward.hpp>
#include <sol/object.hpp>
#include <sol/types.hpp>
#include <stdexcept>

namespace ashfault {
Script::Script(const std::string &source)
    : m_Initialized(false),
      m_Source(source) {}

Script::~Script() {}

void Script::init(sol::state &lua) {
  m_Environment = sol::environment(lua, sol::create, lua.globals());

  auto script = lua.load(m_Source);
  if (!script.valid()) {
    sol::error err = script;
    SPDLOG_ERROR("Failed to load script: {}", err.what());
    throw std::runtime_error("Invalid script!");
  }

  auto result = script(m_Environment);
  if (!result.valid()) {
    throw std::runtime_error("Invalid script!");
  }

  m_OnUpdate = m_Environment["OnUpdate"];
  m_Environment.set_on(m_OnUpdate);
  m_Initialized = true;
}

bool Script::is_initialized() const { return m_Initialized; }

void Script::on_update(float dt, std::optional<Entity> entity) {
  try {
    m_Environment.clear();
    m_Environment.set_function("GetEntity", [&]() {
      sol::lua_value v = entity
              ? sol::make_object(m_Environment.lua_state(), entity->handle())
              : sol::make_object(m_Environment.lua_state(), sol::nil);
      return v;
    });
    m_Environment.set_on(m_OnUpdate);
    m_OnUpdate(dt);
  } catch (sol::error &e) {
    SPDLOG_ERROR("Error executing update function from lua script: {}",
                 e.what());
  }
}
}  // namespace ashfault
