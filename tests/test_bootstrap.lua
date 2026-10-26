local root = assert(arg[1], "Pass the project root")
local initialized, toggled, on_toggle = 0, 0
local exports = {
    ot0_initialize = function() initialized = initialized + 1 end,
    ot0_toggle = function() toggled = toggled + 1 end,
}
package.loadlib = function(_, name) return assert(exports[name]) end
Key = {F6 = 117}
ExecuteInGameThread = function(fn) fn() end
RegisterKeyBind = function(key, fn) assert(key == Key.F6); on_toggle = fn end
dofile(root .. "/mod/OT0AutoResolve/Scripts/main.lua")
assert(initialized == 1 and toggled == 0)
on_toggle()
assert(toggled == 1 and initialized == 1)
print("Bootstrap scheduling and toggle checks passed")
