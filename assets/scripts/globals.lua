---@meta
---@class Entity

---@class Vec3
---@field x number
---@field y number
---@field z number

---Returns the entity the currently executing script is bound to, if any.
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
