#ifndef ASHFAULT_INPUT_H
#define ASHFAULT_INPUT_H

#include <ashfault/core/window.h>
#include <CLSTL/shared_ptr.h>
#include <CLSTL/array.h>

namespace ashfault {
typedef int Key;
typedef int MouseButton;

class Input {
public:
  Input(clstl::shared_ptr<Window> window);

  bool is_key_down(Key key_code);
  bool is_mouse_pressed(MouseButton button);

  void frame_start();

private:
  clstl::shared_ptr<Window> m_Window;
  clstl::array<bool, GLFW_KEY_LAST> m_Keys;
};
}

#endif
