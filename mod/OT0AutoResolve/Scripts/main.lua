local source = debug.getinfo(1, "S").source:sub(2)
local directory = assert(source:match("^(.*)[/\\]"), "Cannot locate mod scripts")
local library = directory .. "/ot0_native.dll"

local function load_export(name)
    local fn, err = package.loadlib(library, name)
    assert(fn, "OT0AutoResolve native module: " .. tostring(err))
    return fn
end

local initialize = load_export("ot0_initialize")
local toggle = load_export("ot0_toggle")
ExecuteInGameThread(initialize)

RegisterKeyBind(Key.F6, function()
    ExecuteInGameThread(function()
        toggle()
    end)
end)

print("[OT0AutoResolve] Loaded. F6 toggles auto resolve; see OT0AutoResolve.log for build status.")
