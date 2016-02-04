-- test luaext math functions using LuaUnit framework
local lu = require("tests.luaunit")
local luaext = require("luaext")
require("math") -- (for checking namespace)

TestMath = {}

function TestMath:test_namespace()
	-- check namespace compliance
	for _, func in ipairs({"frac", "round", "trunc"}) do
		if luaext._config.USE_LUA_NAMESPACES then
			lu.assertEquals(math[func], luaext.math[func])
		else
			lu.assertNil(math[func])
		end
	end
end

function TestMath:test_frac()
	lu.assertEquals(luaext.math.frac(1.98), 0.98)
	lu.assertEquals(luaext.math.frac(-1.75), -0.75)
	lu.assertEquals(luaext.math.frac(1234), 0)
	lu.assertEquals(luaext.math.frac(0), 0)
end

function TestMath:test_round()
	lu.assertEquals(luaext.math.round(math.pi), 3) -- default: round to integer
	lu.assertEquals(luaext.math.round(math.pi, 2), 3.14)
	lu.assertEquals(luaext.math.round(500, -3), 1000) -- round to thousands, "half up"
end

function TestMath:test_trunc()
	lu.assertEquals(luaext.math.trunc(math.pi), 3)
	lu.assertEquals(luaext.math.trunc(1.98), 1)
	lu.assertEquals(luaext.math.trunc(-1.75), -1)
	lu.assertEquals(luaext.math.trunc(0), 0)
end
