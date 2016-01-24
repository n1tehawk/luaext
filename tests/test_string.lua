-- test luaext string functions using LuaUnit framework
local lu = require("tests.luaunit")
local luaext = require("luaext")
require("string") -- (for checking namespace)

TestString = {}

--[[
function TestString:test_namespace()
	-- check namespace compliance
	for _, func in ipairs({"trunc"}) do
		if luaext._config.USE_LUA_NAMESPACES then
			lu.assertEquals(math[func], luaext.math[func])
		else
			lu.assertNil(math[func])
		end
	end
end
--]]

function TestString:test_lua_escape_pattern()
	lu.assertEquals(luaext.lua_escape_pattern("^$()%.[]*+-?"),
					"%^%$%(%)%%%.%[%]%*%+%-%?")
	local foobar = luaext.lua_escape_pattern("foo\0bar")
	if _VERSION == "Lua 5.1" then
		-- check that NUL chars get escaped too
		lu.assertEquals(foobar, "foo%zbar")
	else
		-- later versions should handle embedded NUL, check it's unchanged
		lu.assertEquals(foobar, "foo\0bar")
	end
	lu.assertStrMatches("foo\0bar", foobar)
end
