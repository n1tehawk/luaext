-- luaext.table module
-- This file is part of the luaext library.

local _M = {}
local luaext_lib = require("luaext_lib")

-- import module functions from C library
_M.keyof = luaext_lib.table_keyof
_M.keys = luaext_lib.table_keys


--[[-- create a "mapping" table.

This function takes a table plus a value (or value function). It returns
a new table where the keys correspond to the elements of `t`, and "map" to
entries derived from `value` (see below).

@tparam table t
the table to process (usually a sequential table)

@param value
determines the entries that will be associated with the new keys. It can either
be a simple value (expression) that will be used for `result[x] = value`,
or a function to receive and process each key, returning the desired element:
`result[x] = value(x)`

@usage
primes_below_20 = table.map({2, 3, 5, 7, 11, 13, 17, 19}, true)
function is_small_prime(n) return primes_below_20[n]; end

table.map({"foo", "baar", "rubberducky", "yo"}, string.len)

table.map({"foo", "baar", "rubberducky", "yo"},
	function(word)
		return (#word == 3 or #word == 4) and "funny" or "not funny"
	end)
]]--
function _M.map(t, value, ...)
	if type(t) ~= "table" then
		error("luaext.table.map: table expected, got " .. type(t))
	end
	-- if value is not a function, create one "on the fly"
	if type(value) ~= "function" then
		-- make "v" a 'constant' upvalue for the result
		local v = value or error("luaext.table.map: value (or function) required")
		value = function() return v; end
	end

	local result = {}
	for _, entry in pairs(t) do
		result[entry] = value(entry, ...)
	end
	return result
end
-- table.map({"foo", "baar", "rubberducky", "yo"}, function(word) return (#word == 3 or #word == 4) and "funny" or "not funny"; end)


-- export (selected) functions to Lua "table" namespace
function _M.register_to_Lua()
	require("table")
	for _, func in ipairs({"keyof", "keys", "map"}) do
		table[func] = _M[func]
	end
end

return _M
