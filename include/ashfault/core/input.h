#ifndef ASHFAULT_INPUT_H
#define ASHFAULT_INPUT_H

#include <ashfault/core/window.h>
#include <memory>
#include <array>

namespace ashfault {
typedef int Key;
typedef int MouseButton;

class Input {
public:
  Input(std::shared_ptr<Window> window);

  bool is_key_down(Key key_code);
  bool is_mouse_pressed(MouseButton button);

  void frame_start();

private:
  std::shared_ptr<Window> m_Window;
  std::array<bool, GLFW_KEY_LAST> m_Keys;
};
}

#endif
