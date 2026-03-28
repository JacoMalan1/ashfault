#ifndef ASHFAULT_EVENT_SCRIPT_ATTACH_H
#define ASHFAULT_EVENT_SCRIPT_ATTACH_H

#include <ashfault/ashfault.h>

#include <string>

#include <ashfault/core/entity.h>
#include <ashfault/core/event.h>

namespace ashfault {
class ASHFAULT_API ScriptAttachEvent : public Event {
public:
  ScriptAttachEvent(Entity entity, const std::string &script_path);

  static Event::EventType static_type();

  EventType event_type() const override;

  Entity entity() const;
  const std::string &script_path() const;

private:
  Entity m_Entity;
  std::string m_ScriptPath;
};
}  // namespace ashfault

#endif
