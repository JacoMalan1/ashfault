#ifndef ASHFAULT_EDITOR_UI_LAYER_H
#define ASHFAULT_EDITOR_UI_LAYER_H

#include <ashfault/core/event.h>
#include <ashfault/core/layer.h>

namespace ashfault {
class EditorUiLayer : public Layer {
public:
  EditorUiLayer();
  ~EditorUiLayer();

  void on_update(float dt) override;
  void on_render() override;
  void on_event(Event &event) override;
  void on_imgui_render() override;

private:
};
}

#endif
