-- test luaext table functions using LuaUnit framework
local lu = require("tests.luaunit")
local luaext = require("luaext")
require("table") -- (for checking namespace)

TestTable = {}

function TestTable:test_namespace()
	-- check namespace compliance
	for _, func in ipairs({"keyof", "keys", "map"}) do
		if luaext._config.USE_LUA_NAMESPACES then
			lu.assertEquals(table[func], luaext.table[func])
		else
			lu.assertNil(table[func])
		end
	end
end

function TestTable:test_keyof()
	local t = {"foo", nil, 42, "bar"}
	lu.assertEquals(luaext.table.keyof(t, "bar"), 4)
	lu.assertEquals(luaext.table.keyof(t, "foo"), 1)
	lu.assertNil(luaext.table.keyof(t, "foobar"))
	lu.assertEquals(luaext.table.keyof(t, 42), 3)
end

function TestTable:test_keys()
	local function assertTableContent(actual, expected)
		lu.assertEquals(#actual, #expected)
		for k, v in pairs(expected) do
			lu.assertNotNil(luaext.table.keyof(actual, v)) -- "v is in actual"
		end
	end

	local t = {XoXo=5, foo=2, bar=1, xfoo=3, barX=4, xXx=6}
	-- all keys from t (unsorted)
	local keys, count = luaext.table.keys(t)
	assertTableContent(keys, {"barX", "foo", "XoXo", "xfoo", "xXx", "bar"})
	lu.assertEquals(count, 6)
	-- all keys from t (sorted in standard table.sort() order)
	keys, count = luaext.table.keys(t, true)
	lu.assertEquals(keys, {"XoXo", "bar", "barX", "foo", "xXx", "xfoo"})
	lu.assertEquals(count, 6)

	-- custom sort on keys
	keys, count = luaext.table.keys({6, 4, 5}, function(a, b) return b < a; end)
	lu.assertEquals(keys, {3, 2, 1})
	lu.assertEquals(count, 3)

	-- return keys of t containing an 'x', unsorted
	keys, count = luaext.table.keys(t, nil, string.match, "[xX]")
	assertTableContent(keys, {"barX", "XoXo", "xfoo", "xXx"})
	lu.assertEquals(count, 4)
	-- return keys of t containing an 'x', sorted in standard table.sort() order
	keys, count = luaext.table.keys(t, true, string.match, "[xX]")
	lu.assertEquals(keys, {"XoXo", "barX", "xXx", "xfoo"})
	lu.assertEquals(count, 4)

	t = {xoxo=5, foo=2, bar=1, xfoo=3, barx=4, xxx=6}
	-- return keys of t containing an 'x', sorted by custom function (descending order)
	keys, count = luaext.table.keys(t, function(a, b) return a > b; end, string.find, "x", 1, true)
	lu.assertEquals(keys, {"xxx", "xoxo", "xfoo", "barx"})
	lu.assertEquals(count, 4)
	keys, count = luaext.table.keys(t, true, string.find, "^ba")
	lu.assertEquals(keys, {"bar", "barx"})
	lu.assertEquals(count, 2)

	-- luaext.table.keys() with a filter function (and sorting)
	t = luaext.table.map({"foo", "bar", "foobar", "fubar", "bart"}, "don't care")
	keys, count = luaext.table.keys(t, true, string.find, "^f")
	lu.assertEquals(keys, {"foo", "foobar", "fubar"})
	lu.assertEquals(count, 3)
end

function TestTable:test_map()
	local t = luaext.table.map(
		{"foo", "baar", "rubberducky", "yo"},
		function(word, result_t, result_f)
			return (#word == 3 or #word == 4) and result_t or result_f
		end,
		"funny", "not funny")
	lu.assertEquals(t.yo, "not funny")
	lu.assertEquals(t.foo, "funny")
	lu.assertEquals(t.baar, "funny")
	lu.assertEquals(t.rubberducky, "not funny")
end
