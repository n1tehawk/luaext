--[[--
The luaext core module.


Usually all you need to do is a
	require("luaext")
from your project. With the default [configuration](#_config), this creates a
global variable` luaext` (holding the module table), and also includes the
various submodules.

If you prefer less 'intrusive' operation, you may change the configuration
accordingly, and use something like
	local luaext = require("luaext")
Submodules are always available within the module table, so e.g. to access the
mathematical functions you can use `luaext.math`, etc.

---
@module luaext
]]
local _M = {} -- the module table

--[[--
luaext configuration - predefine `LUAEXT_CONFIG` or edit these settings

@field EXPORT_MODULE_AS_GLOBAL
&nbsp;if set (`true`), luaext will export the module table as global var "luaext"

@field EXPORT_GLOBAL_FUNCTIONS
&nbsp;if set, luaext will export some function names to the global namespace
(e.g. `printf`, `error_fmt`, ...)

@field USE_LUA_NAMESPACES
&nbsp;if set, luaext (submodules) will use / add to predefined Lua namespaces
(like `math`, `string`, `table`, ...)

@table _config
]]
_M._config = rawget(_G, "LUAEXT_CONFIG") or {
	EXPORT_MODULE_AS_GLOBAL = true,
	EXPORT_GLOBAL_FUNCTIONS = true,
	USE_LUA_NAMESPACES = true,
}

-- which submodules to load (normally you'd want them all)
-- local submodules = {"math", "string", "table"},
local submodules = {"math", "string", "table"}

-- retrieve the low-level C module and import all its functions to this module
-- TODO: we might have to be more strict about this (ignore submodule functions)
local luaext_lib = require("luaext_lib")
for k, v in pairs(luaext_lib) do
	if type(v) == "function" then
		_M[k] = luaext_lib[k]
	end
end

-- import all the (Lua) submodules
for _, submodule in ipairs(submodules) do
	local module = require("luaext." .. submodule)
	_M[submodule] = module
	if _M._config.USE_LUA_NAMESPACES then
		module.register_to_Lua() -- register to Lua namespaces
	end
	if _M._config.EXPORT_GLOBAL_FUNCTIONS and module.register_globals then
		module.register_globals() -- (optionally) register globals
	end
end

if _M._config.EXPORT_MODULE_AS_GLOBAL then
	luaext = _M
end
if _M._config.EXPORT_GLOBAL_FUNCTIONS then
	-- export specific functions to the global namespace
	for _, func in ipairs(
		{"empty", "error_fmt", "printf"}
	) do
		if not _M[func] then
			luaext_lib.error_fmt('FAILED to export "%s": no such function', func)
		end
		_G[func] = _M[func]
	end
end

return _M
