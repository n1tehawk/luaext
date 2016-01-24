local _M = {} -- the module table

-- luaext configuration - predefine LUAEXT_CONFIG or edit these settings
_M._config = rawget(_G, "LUAEXT_CONFIG") or {
	EXPORT_MODULE_AS_GLOBAL = true,
	EXPORT_GLOBAL_FUNCTIONS = true,
	USE_LUA_NAMESPACES = true,
}

-- which submodules to load (normally you'd want them all)
-- local submodules = {"math", "string", "table"},
local submodules = {"math", "table"}

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
	_M[submodule] = require("luaext." .. submodule)
	if _M._config.USE_LUA_NAMESPACES then
		_M[submodule].register_to_Lua()
	end
end

if _M._config.EXPORT_MODULE_AS_GLOBAL then
	luaext = _M
end
if _M._config.EXPORT_GLOBAL_FUNCTIONS then
	-- export specific functions to the global namespace
	for _, func in ipairs(
		{"empty", "error_fmt", "lua_escape_pattern", "printf"}
	) do
		_G[func] = _M[func]
	end
end

return _M
