#include <ashfault/core/event/script_attach.h>

namespace ashfault {
ScriptAttachEvent::ScriptAttachEvent(Entity entity, const std::string &path)
    : m_Entity(entity), m_ScriptPath(path) {}

Event::EventType ScriptAttachEvent::static_type() {
  return Event::ScriptAttach;
}

Event::EventType ScriptAttachEvent::event_type() const {
  return Event::ScriptAttach;
}

Entity ScriptAttachEvent::entity() const { return m_Entity; }

const std::string &ScriptAttachEvent::script_path() const {
  return m_ScriptPath;
}
}  // namespace ashfault
