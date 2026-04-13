function OnSceneStart()
	local e = GetEntity()
	print(string.format("Initializing entity %d", e))
	local right = Input.RegisterAction("right")
	Input.BindAction(right, { { Key.D } })
	Input.RegisterAction("left")
	Input.BindAction("left", { { Key.A } })
	Input.RegisterAction("back")
	Input.BindAction("back", { { Key.W } })
	Input.RegisterAction("forward")
	Input.BindAction("forward", { { Key.S } })
	Input.RegisterAction("up")
	Input.BindAction("up", { { Key.Space } })
	Input.RegisterAction("down")
	Input.BindAction("down", { { Key.LeftShift, Key.Space } })

	if e ~= nil then
		local transform = Scene.GetComponent(e, Transform)
		if transform ~= nil then
			transform.position.x = 0
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
			-- print("hello")
			-- print(string.format("GetAction value: %s", Input.GetAction("forward")))
			right = Input.GetActionId("right")
			if Input.GetAction(right) then
				transform.position.x = transform.position.x + 1.0 * dt
			end
			if Input.GetAction("left") then
				transform.position.x = transform.position.x - 1.0 * dt
			end
			if Input.GetAction("back") then
				transform.position.z = transform.position.z + 1.0 * dt
			end
			if Input.GetAction("forward") then
				transform.position.z = transform.position.z - 1.0 * dt
			end
			if Input.GetAction("up") then
				transform.position.y = transform.position.y + 1.0 * dt
			end
			if Input.GetAction("down") then
				transform.position.y = transform.position.y - 1.0 * dt
			end
		end
	end
end
