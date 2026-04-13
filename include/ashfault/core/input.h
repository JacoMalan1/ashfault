#ifndef ASHFAULT_INPUT_H
#define ASHFAULT_INPUT_H

#include <ashfault/ashfault.h>
#include <ashfault/core/window.h>

#include <memory>

namespace ashfault {
typedef std::uint32_t ActionId;
enum class Key : int {
  Unknown = -1,
  Space = 32,
  Apostrophe = 39,
  Comma = 44,
  Minus = 45,
  Period = 46,
  Slash = 47,
  Num0 = 48,
  Num1 = 49,
  Num2 = 50,
  Num3 = 51,
  Num4 = 52,
  Num5 = 53,
  Num6 = 54,
  Num7 = 55,
  Num8 = 56,
  Num9 = 57,
  Semicolon = 59,
  Equal = 61,
  A = 65,
  B = 66,
  C = 67,
  D = 68,
  E = 69,
  F = 70,
  G = 71,
  H = 72,
  I = 73,
  J = 74,
  K = 75,
  L = 76,
  M = 77,
  N = 78,
  O = 79,
  P = 80,
  Q = 81,
  R = 82,
  S = 83,
  T = 84,
  U = 85,
  V = 86,
  W = 87,
  X = 88,
  Y = 89,
  Z = 90,
  LeftBracket = 91,
  Backslash = 92,
  RightBracket = 93,
  GraveAccent = 96,
  World1 = 161,
  World2 = 162,

  Escape = 256,
  Enter = 257,
  Tab = 258,
  Backspace = 259,
  Insert = 260,
  Delete = 261,
  Right = 262,
  Left = 263,
  Down = 264,
  Up = 265,
  PageUp = 266,
  PageDown = 267,
  Home = 268,
  End = 269,
  CapsLock = 280,
  ScrollLock = 281,
  NumLock = 282,
  PrintScreen = 283,
  Pause = 284,
  F1 = 290,
  F2 = 291,
  F3 = 292,
  F4 = 293,
  F5 = 294,
  F6 = 295,
  F7 = 296,
  F8 = 297,
  F9 = 298,
  F10 = 299,
  F11 = 300,
  F12 = 301,
  F13 = 302,
  F14 = 303,
  F15 = 304,
  F16 = 305,
  F17 = 306,
  F18 = 307,
  F19 = 308,
  F20 = 309,
  F21 = 310,
  F22 = 311,
  F23 = 312,
  F24 = 313,
  F25 = 314,

  KP0 = 320,
  KP1 = 321,
  KP2 = 322,
  KP3 = 323,
  KP4 = 324,
  KP5 = 325,
  KP6 = 326,
  KP7 = 327,
  KP8 = 328,
  KP9 = 329,
  KPDecimal = 330,
  KPDivide = 331,
  KPMultiply = 332,
  KPSubtract = 333,
  KPAdd = 334,
  KPEnter = 335,
  KPEqual = 336,

  LeftShift = 340,
  LeftControl = 341,
  LeftAlt = 342,
  LeftSuper = 343,
  RightShift = 344,
  RightControl = 345,
  RightAlt = 346,
  RightSuper = 347,
  Menu = 348,

  Mouse1 = 500,
  Mouse2 = 501,
  Mouse3 = 502,
  Mouse4 = 503,
  Mouse5 = 504,
  Mouse6 = 505,
  Mouse7 = 506,
  Mouse8 = 507,
  MouseLeft = 500,
  MouseRight = 501,
  MouseMiddle = 502
};

struct KeyCombination {
  std::vector<Key> keys;
};
class ASHFAULT_API Input {
public:
  static void init(std::shared_ptr<Window> window);
  static ActionId register_action(const std::string &action_name);
  static ActionId register_action(
      const std::string &action_name,
      const std::vector<KeyCombination> &key_combinations);
  static void bind_action(ActionId action,
                          const std::vector<KeyCombination> &key_combinations);
  static void bind_action(std::string action,
                          const std::vector<KeyCombination> &key_combinations);
  static void unbind_action(ActionId action);
  static void unbind_action(std::string action);
  static void add_modifier_key(Key key);
  static void remove_modifier_key(Key key);
  static void clear_all_modifiers();

  static bool get_action(ActionId action_id);
  static bool get_action(const std::string &action);
  static ActionId get_action_id(const std::string &action_name);

  void frame_start();
};
}  // namespace ashfault

#endif
