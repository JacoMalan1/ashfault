---@meta
---@class Entity

---@class Key
---@field Unknown number
---@field Space number
---@field Apostrophe number
---@field Comma number
---@field Minus number
---@field Period number
---@field Slash number
---@field Num0 number
---@field Num1 number
---@field Num2 number
---@field Num3 number
---@field Num4 number
---@field Num5 number
---@field Num6 number
---@field Num7 number
---@field Num8 number
---@field Num9 number
---@field Semicolon number
---@field Equal number
---@field A number
---@field B number
---@field C number
---@field D number
---@field E number
---@field F number
---@field G number
---@field H number
---@field I number
---@field J number
---@field K number
---@field L number
---@field M number
---@field N number
---@field O number
---@field P number
---@field Q number
---@field R number
---@field S number
---@field T number
---@field U number
---@field V number
---@field W number
---@field X number
---@field Y number
---@field Z number
---@field LeftBracket number
---@field Backslash number
---@field RightBrackethtBracket number
---@field GraveAccent number
---@field World1 number
---@field World2 number
---@field Escape number
---@field Enter number
---@field Tab number
---@field Backspace number
---@field Insert number
---@field Delete number
---@field Right number
---@field Left number
---@field Down number
---@field Up number
---@field PageUp number
---@field PageDown number
---@field Home number
---@field End number
---@field CapsLock number
---@field ScrollLock number
---@field NumLock number
---@field PrintScreen number
---@field Pause number
---@field F1 number
---@field F2 number
---@field F3 number
---@field F4 number
---@field F5 number
---@field F6 number
---@field F7 number
---@field F8 number
---@field F9 number
---@field F10 number
---@field F11 number
---@field F12 number
---@field F13 number
---@field F14 number
---@field F15 number
---@field F16 number
---@field F17 number
---@field F18 number
---@field F19 number
---@field F20 number
---@field F21 number
---@field F22 number
---@field F23 number
---@field F24 number
---@field F25 number
---@field KP0 number
---@field KP1 number
---@field KP2 number
---@field KP3 number
---@field KP4 number
---@field KP5 number
---@field KP6 number
---@field KP7 number
---@field KP8 number
---@field KP9 number
---@field KPDecimal number
---@field KPDivide number
---@field KPMultiply number
---@field KPSubtract number
---@field KPAdd number
---@field KPEnter number
---@field KPEqual number
---@field LeftShift number
---@field LeftControl number
---@field LeftAlt number
---@field LeftSuper number
---@field RightShift number
---@field RightControl number
---@field RightAlt number
---@field RightSuper number
---@field Menu number
---@field Mouse1 number
---@field Mouse2 number
---@field Mouse3 number
---@field Mouse4 number
---@field Mouse5 number
---@field Mouse6 number
---@field Mouse7 number
---@field Mouse8 number
---@field MouseLeft number
---@field MouseRight number
---@field MouseMiddle number
Key = {}

---@class Vec3
---@field x number
---@field y number
---@field z number

---Returns the entity the currently executing script is bound to, if any.
---
---@return Entity?
function GetEntity() end

---@param msg string
function print(msg) end

---@class Scene
Scene = {}

---@class Transform
---@field position Vec3
---@field rotation Vec3
---@field scale Vec3
Transform = {}

---@return Transform
function Transform.new() end

---@class Tag
---@field tag string
Tag = {}

---@return Tag
function Tag.new() end

---@class PointLight
---@field position Vec3
---@field color Vec3
PointLight = {}

---@return PointLight
function PointLight.new() end

---@class DirectionalLight
---@field direction Vec3
---@field color Vec3
DirectionalLight = {}

---@return DirectionalLight
function DirectionalLight.new() end

---Returns the entity the currently executing script is bound to, if any.
---
---@example
---```lua
---local entity = GetEntity()
---if entity ~= nil then
---  local transform = Scene.GetComponent(entity, Transform)
---  if transform ~= nil then
---    transform.position.x = transform.position.x + 0.01
---  end
---end
---```
---@generic T
---@param entity Entity
---@param type T
---@return T|nil
function Scene.GetComponent(entity, type) end

---Adds a component to an existing entity.
---
---@generic T
---@param entity Entity
---@param type T
---@param component T
function Scene.AddComponent(entity, type, component) end

---@return Entity[]|nil
function Scene.GetEntities() end

---@class Input
Input = {}

---@alias actionID integer
---@alias KeyCombination integer[]

---Registers an action name and returns the corresponding action_id
---
---@param actionName string
---@return actionID
function Input.RegisterAction(actionName) end

---Binds a sequence of key combinations to an action
---
---@param action string|actionID
---@param keyCombinations KeyCombination[]
function Input.BindAction(action, keyCombinations) end

---Removes all key bindings from an action
---
---@param action string|actionID
function Input.UnbindAction(action) end

---Returns the actionID of an action with the provided name
---
---@param actionName string
function Input.GetActionId(actionName) end

---Checks if any of an action's bound keyCombinations have been pressed in the current frame
---
---@param action string|actionID
---@return boolean
function Input.GetAction(action) end

---Adds a key to the list of modifier keys
---
---@param key Key
function Input.AddModifierKey(key) end

---Removes a key from the list of modifier keys
---
---@param key Key
function Input.RemoveModifierKey(key) end

---Removes all keys from the list of modifier keys
---
---@param key Key
function Input.ClearAllModifiers(key) end
