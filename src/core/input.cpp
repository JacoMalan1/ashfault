#include <GLFW/glfw3.h>
#include <ashfault/core/input.h>

#include <cstring>

namespace ashfault {
Input::Input(std::shared_ptr<Window> window) : m_Window(window) {
  std::memset(this->m_Keys.data(), 0, this->m_Keys.size() * sizeof(bool));
  window->set_key_callback([&](Window &, int key, int, int action, int) {
    if (action == GLFW_PRESS) {
      this->m_Keys[key] = true;
    } else if (action == GLFW_RELEASE) {
      this->m_Keys[key] = false;
    }
  });
}

bool Input::is_key_down(Key key) { return this->m_Keys[key]; }

bool Input::is_mouse_pressed(MouseButton button) {
  return glfwGetMouseButton(this->m_Window->handle(), static_cast<int>(button));
}

void Input::frame_start() {
  std::memset(this->m_Keys.data(), 0, this->m_Keys.size() * sizeof(bool));
}
}  // namespace ashfault
