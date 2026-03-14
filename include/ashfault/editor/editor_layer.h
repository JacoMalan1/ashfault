#ifndef ASHFAULT_EDITOR_LAYER_H
#define ASHFAULT_EDITOR_LAYER_H

#include <ashfault/core/layer.h>

namespace ashfault {
class EditorLayer : public Layer {
public:
  EditorLayer();
  ~EditorLayer();

  void on_attach() override;
  void on_detach() override;

  void on_update(float dt) override;
  void on_render() override;
  void on_event(Event &event) override;

private:
};
}

#endif
