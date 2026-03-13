#ifndef ASHFAULT_LAYER_H
#define ASHFAULT_LAYER_H

#include <ashfault/core/event.h>
#include <ashfault/ashfault.h>

namespace ashfault {
class ASHFAULT_API Layer {
public:
  Layer();
  virtual ~Layer() = default;

  virtual void on_attach() = 0;
  virtual void on_detach() = 0;

  virtual void on_update(float dt) = 0;
  virtual void on_render() = 0;
  virtual void on_event(Event &event) = 0;

  bool is_enabled() const;
  void set_enabled(bool enabled);

protected:
  bool m_Enabled;
};
}

#endif
