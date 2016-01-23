-- luaext.math module
-- This file is part of the luaext library.

local _M = {}
local luaext_lib = require("luaext_lib")

-- import module functions from C library
_M.trunc = luaext_lib.math_trunc


-- export (selected) functions to Lua "table" namespace
function _M.register_to_Lua()
	require("math")
	for _, func in ipairs({"trunc"}) do
		math[func] = _M[func]
	end
end

return _M
