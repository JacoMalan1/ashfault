function OnUpdate(dt)
  local e = GetEntity()
  if e ~= nil then
    local transform = Scene.GetComponent(e, Transform)
    if transform ~= nil then
      transform.position.x = transform.position.x + 1 * dt
    end
  end
end
