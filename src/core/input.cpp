#include <GLFW/glfw3.h>
#include <ashfault/core/input.h>
#include <spdlog/spdlog.h>

#include <cstring>

namespace ashfault {
struct InputData {
  std::unordered_map<std::string, ActionId> action_names;
  std::unordered_map<ActionId, std::vector<KeyCombination>> actions;
  ActionId next_id;
  std::shared_ptr<Window> window;
  std::array<bool, 507> keys;
  std::vector<Key> modifiers;
};

static InputData s_Data;
void Input::init(std::shared_ptr<Window> window) {
  s_Data.window = window;
  s_Data.next_id = 0;
  s_Data.action_names = std::unordered_map<std::string, ActionId>();
  s_Data.actions = std::unordered_map<ActionId, std::vector<KeyCombination>>();
  s_Data.modifiers =
      std::vector<Key>({Key::LeftShift, Key::LeftAlt, Key::LeftControl});

  std::memset(s_Data.keys.data(), 0, s_Data.keys.size() * sizeof(bool));
  window->set_key_callback([&](Window &, int key, int, int action, int) {
    if (action == GLFW_PRESS) {
      s_Data.keys[key] = true;
    } else if (action == GLFW_RELEASE) {
      s_Data.keys[key] = false;
    }
  });
  window->set_mouse_callback([&](Window &, int button, int action, int) {
    if (action == GLFW_PRESS) {
      s_Data.keys[button + 500] = true;
    } else if (action == GLFW_RELEASE) {
      s_Data.keys[button + 500] = false;
    }
  });
}

ActionId Input::register_action(const std::string &action_name) {
  s_Data.action_names.emplace(action_name, s_Data.next_id);
  return s_Data.next_id++;
}
ActionId Input::register_action(
    const std::string &action_name,
    const std::vector<KeyCombination> &key_combinations) {
  s_Data.action_names.emplace(action_name, s_Data.next_id);
  s_Data.actions.emplace(s_Data.next_id, key_combinations);
  return s_Data.next_id++;
}
void Input::bind_action(ActionId action,
                        const std::vector<KeyCombination> &key_combinations) {
  s_Data.actions.insert_or_assign(action, key_combinations);
}

void Input::bind_action(std::string action,
                        const std::vector<KeyCombination> &key_combinations) {
  ActionId data = get_action_id(action);
  s_Data.actions.insert_or_assign(data, key_combinations);
}
bool Input::get_action(ActionId action_id) {
  std::vector<KeyCombination> action_keys = s_Data.actions.at(action_id);

  for (const KeyCombination &key_combination : action_keys) {
    std::vector<Key> modifiers_no_press = s_Data.modifiers;
    bool action_condition_fulfilled = true;
    for (const Key &key : key_combination.keys) {
      if (s_Data.keys[static_cast<int>(key)] == false) {
        action_condition_fulfilled = false;
        break;
      } else {
        std::erase(modifiers_no_press, key);
      }
    }
    if (action_condition_fulfilled) {
      for (const Key &key : modifiers_no_press) {
        if (s_Data.keys[static_cast<int>(key)] == true) {
          return false;
        }
      }
      return true;
    }
  }
  return false;
}

bool Input::get_action(const std::string &action) {
  std::vector<KeyCombination> action_keys =
      s_Data.actions.at(get_action_id(action));

  for (const KeyCombination &key_combination : action_keys) {
    std::vector<Key> modifiers_no_press = s_Data.modifiers;
    bool action_condition_fulfilled = true;
    for (const Key &key : key_combination.keys) {
      if (s_Data.keys[static_cast<int>(key)] == false) {
        action_condition_fulfilled = false;
        break;
      } else {
        std::erase(modifiers_no_press, key);
      }
    }
    if (action_condition_fulfilled) {
      for (const Key &key : modifiers_no_press) {
        if (s_Data.keys[static_cast<int>(key)] == true) {
          return false;
        }
      }
      return true;
    }
  }
  return false;
}

ActionId Input::get_action_id(const std::string &action_name) {
  return s_Data.action_names.at(action_name);
}

void Input::frame_start() {
  std::memset(s_Data.keys.data(), 0, s_Data.keys.size() * sizeof(bool));
}

}  // namespace ashfault
