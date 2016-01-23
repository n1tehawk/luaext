-- test luaext math functions using LuaUnit framework
local lu = require("tests.luaunit")
local luaext = require("luaext")
require("math") -- (for checking namespace)

TestMath = {}

function TestMath:test_namespace()
	-- check namespace compliance
	for _, func in ipairs({"trunc"}) do
		if luaext._config.USE_LUA_NAMESPACES then
			lu.assertEquals(math[func], luaext.math[func])
		else
			lu.assertNil(math[func])
		end
	end
end

function TestMath:test_trunc()
	lu.assertEquals(luaext.math.trunc(math.pi), 3)
	lu.assertEquals(luaext.math.trunc(1.98), 1)
	lu.assertEquals(luaext.math.trunc(-1.75), -1)
	lu.assertEquals(luaext.math.trunc(0), 0)
end
