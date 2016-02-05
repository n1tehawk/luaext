--[[--
String functions. This file is part of the *luaext* library.

If `USE_LUA_NAMESPACES` is [configured](luaext.html#_config), the module
functions will also get exported to the standard` string` namespace.

@module luaext.string
]]

local _M = {}
local luaext_lib = require("luaext_lib")

-- import "low-level" module functions from C library
_M.escape_lua_pattern = luaext_lib.string_escape_lua_pattern
--_M.trunc = luaext_lib.math_trunc


--[[-- center a string, filling in chars up to a given width.
The function will first extend the given string by prepending `pad_left` and
appending `pad_right`, respectively. After that, if the string length is still
smaller than `width`, it will add a suitable amount of `fillChar` characters,
so that the string is kept centered, and the result ends up with the requested
`width`.

@usage
print("|" .. luaext.string.center("--> center <--", 30)          .. "|")
print("|" .. luaext.string.center("center", 30, "-", "> ", " <") .. "|")
print("|" .. luaext.string.center("Bruce Banner", 30, "*", " ")  .. "|")

@function center
@tparam string str the string to process
@tparam number width the desired length (number of characters) for the result
@tparam ?string fillChar character to use when filling up the string to the
final (centered) result. defaults to space (" ")
@tparam ?string pad_left will be prepended to string before `fillChar` gets
applied. defaults to empty string ("")
@tparam ?string pad_right will be appended to string before `fillChar` gets
applied. defaults to `pad_left`
]]
function _M.center(str, width, fillChar, pad_left, pad_right)
	if type(width) == "number" then
		fillChar = string.sub(fillChar or " ", 1, 1)
		pad_left = pad_left or ""
		pad_right = pad_right or pad_left
		str = string.sub(pad_left .. str .. pad_right, 1, width)
		len = #str
		if len < width then
			str = str .. fillChar:rep((width - len) / 2)
			len = #str
			str = fillChar:rep(width - len) .. str
		end
	end
	return str
end

-- export (selected) functions to Lua "string" namespace
function _M.register_to_Lua()
	require("string")
	for _, func in ipairs({"escape_lua_pattern"}) do
		assert(_M[func] ~= nil)
		string[func] = _M[func]
	end
end

-- upon request: make a few, selected functions available globally
function _M.register_globals()
	_G.escape_lua_pattern = _M.escape_lua_pattern
end

return _M
