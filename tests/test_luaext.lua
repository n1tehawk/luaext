-- test (core) luaext functions using LuaUnit framework
local lu = require("tests.luaunit")
local LUAJIT = rawget(_G, "jit") ~= nil

require("luaext")

local ext = rawget(_G, "luaext")
if ext then
	-- found global var, make sure that matches luaext configuration
	lu.assertTrue(ext._config.EXPORT_MODULE_AS_GLOBAL)
else
	ext = require("luaext")
	lu.assertFalse(ext._config.EXPORT_MODULE_AS_GLOBAL)
end


TestLuaExt = {} -- test class

function TestLuaExt:test_crossTypeCompare()
	lu.assertEquals(ext.crossTypeCompare(true, "foo"), -1)
	lu.assertEquals(ext.crossTypeCompare("foo", true), 1)
	lu.assertEquals(ext.crossTypeCompare(false, "foo"), -1)
	lu.assertEquals(ext.crossTypeCompare("foo", false), 1)
	lu.assertEquals(ext.crossTypeCompare("foo"), 1)
	lu.assertEquals(ext.crossTypeCompare(nil, "foo"), -1)
	lu.assertEquals(ext.crossTypeCompare("foo", "bar"), 1)
	lu.assertEquals(ext.crossTypeCompare("bar", "foo"), -1)
	lu.assertEquals(ext.crossTypeCompare("42", 42), 1)
	lu.assertEquals(ext.crossTypeCompare(42, "42"), -1)
	lu.assertEquals(ext.crossTypeCompare(true, false), 1)
	lu.assertEquals(ext.crossTypeCompare(false, true), -1)

	lu.assertEquals(ext.crossTypeCompare(), 0) -- nil == nil
	lu.assertEquals(ext.crossTypeCompare(nil, nil), 0)
	lu.assertEquals(ext.crossTypeCompare("foo", "foo"), 0)
	lu.assertEquals(ext.crossTypeCompare(true, true), 0)
	lu.assertEquals(ext.crossTypeCompare(false, false), 0)

	local a, b = {}, {} -- two (different!) tables
	lu.assertNotEquals(ext.crossTypeCompare(a, b), 0)
	lu.assertNotEquals(ext.crossTypeCompare(b, a), 0)
	lu.assertEquals(ext.crossTypeCompare(a, a), 0)
	lu.assertEquals(ext.crossTypeCompare(b, b), 0)
end

function TestLuaExt:test_crossTypeSort()
	local t = {
		"foo", nil, "bar", 2, {}, true, 3, 5, "bear", {}, false, "foobar"
	}
	table.sort(t, ext.crossTypeSort)

	--for k, v in pairs(t) do print(k, v); end
	lu.assertEquals(t[1], nil)
	lu.assertEquals(t[2], 2)
	lu.assertEquals(t[3], 3)
	lu.assertEquals(t[4], 5)
	lu.assertEquals(t[5], false)
	lu.assertEquals(t[6], true)
	lu.assertEquals(t[7], "bar")
	lu.assertEquals(t[8], "bear")
	lu.assertEquals(t[9], "foo")
	lu.assertEquals(t[10], "foobar")
	lu.assertEquals(type(t[11]), "table")
	lu.assertEquals(type(t[12]), "table")
end

function TestLuaExt:test_empty()
	lu.assertNil(ext.empty()) -- no arguments, no result (= nil)

	-- "empty"
	local function assertEmpty(...) lu.assertTrue(ext.empty(...)); end
	assertEmpty(nil)
	assertEmpty("")
	assertEmpty({})
	if LUAJIT then
		-- LuaJIT detected, check proper handling of <cdata>
		assertEmpty(require("ffi").cast("void*", 0))
	end

	-- not empty
	local function assertNotEmpty(...) lu.assertFalse(ext.empty(...)); end
	assertNotEmpty(true)
	assertNotEmpty(false)
	assertNotEmpty("foobar")
	assertNotEmpty(42)
	assertNotEmpty({42})
	assertNotEmpty({foo = "bar"})
	if LUAJIT then
		assertNotEmpty(require("ffi").cast("void*", 1))
	end

	-- multiple arguments
	local t = {"foo", {}, "bar"}
	lu.assertEquals({ext.empty(t, t[2], t[3], t[5])},
					{false, true, false, true}) -- (no value at index 5)

	if ext._config.EXPORT_GLOBAL_FUNCTIONS then
		lu.assertEquals(rawget(_G, "empty"), ext.empty)
	else
		lu.assertNil(rawget(_G, "empty"))
	end
end
