-- test luaext string functions using LuaUnit framework
local lu = require("tests.luaunit")
local luaext = require("luaext")

TestString = {}

function TestString:test_namespace()
	local namespace = "string"
	local funcs = {"escape_lua_pattern"}
	local globals = {"escape_lua_pattern"}

	-- check namespace compliance
	local module = require(namespace)
	for _, func in ipairs(funcs) do
		if luaext._config.USE_LUA_NAMESPACES then
			if not module[func] then
				luaext.error_fmt("Expected function %s.%s NOT FOUND", namespace, func)
			end
			lu.assertEquals(module[func], luaext.string[func])
		else
			lu.assertNil(module[func])
		end
	end
	-- global symbols
	for _, func in ipairs(globals) do
		if luaext._config.EXPORT_GLOBAL_FUNCTIONS then
			lu.assertNotNil(rawget(_G, func))
		else
			lu.assertNil(rawget(_G, func))
		end
	end
end

function TestString:test_escape_pattern()
	lu.assertEquals(luaext.string.escape_lua_pattern("^$()%.[]*+-?"),
					"%^%$%(%)%%%.%[%]%*%+%-%?")
	local foobar = luaext.string.escape_lua_pattern("foo\0bar")
	if _VERSION == "Lua 5.1" then
		-- check that NUL chars get escaped too
		lu.assertEquals(foobar, "foo%zbar")
	else
		-- later versions should handle embedded NUL, check it's unchanged
		lu.assertEquals(foobar, "foo\0bar")
	end
	lu.assertStrMatches("foo\0bar", foobar)
end
