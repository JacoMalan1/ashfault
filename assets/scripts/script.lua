function OnSceneStart()
  local e = GetEntity()
  print(string.format("Initializing entity %d", e))
  if e ~= nil then
    local transform = Scene.GetComponent(e, Transform)
    if transform ~= nil then
      transform.position.x = 0
      transform.position.y = 0
      transform.position.z = 0
    else
      print(string.format("Entity %d does not have a transform, creating one...", e))
      local t = Transform.new()
      t.scale.x = 1
      t.scale.y = 1
      t.scale.z = 1
      Scene.AddComponent(e, Transform, t)
    end
  end
end

function OnUpdate(dt)
  local e = GetEntity()
  if e ~= nil then
    local transform = Scene.GetComponent(e, Transform)
    if transform ~= nil then
      transform.position.x = transform.position.x + 1.0 * dt
    end
  end
end
