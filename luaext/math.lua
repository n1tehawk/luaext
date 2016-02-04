--[[--
Mathematical functions. This file is part of the *luaext* library.

If `USE_LUA_NAMESPACES` is [configured](luaext.html#_config), the module
functions will also get exported to the standard` math` namespace.

@module luaext.math
]]

local _M = {}
local luaext_lib = require("luaext_lib")
require("math")

-- import "low-level" module functions from C library
_M.frac = luaext_lib.math_frac
_M.trunc = luaext_lib.math_trunc


-- predefined powers of ten (useful for math.round)
local POW10 = {[0] = 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000}

--[[-- rounds a number.
This function will perform rounding of a value to the given number of decimal
places (precision), using a "round half up" algorithm
(see [Wikipedia](https://en.wikipedia.org/wiki/Rounding#Round_half_up)).
Note: It's even possible to use negative values of `decimals` to round to
higher powers of ten.

@usage
assert(luaext.math.round(math.pi, 2) == 3.14)
assert(luaext.math.round(500, -3) == 1000) -- round to 1E3 = thousands

@function round
@tparam number n the value to round
@tparam ?number decimals precision (number of decimal places) to keep after
rounding. defaults to 0, which rounds to an integer value.
]]
function _M.round(number, decimals)
	decimals = math.floor(decimals or 0)
	local factor = POW10[decimals]
	if not factor then
		factor = 10 ^ decimals
		POW10[decimals] = factor -- (cache result)
	end
	return _M.trunc(number * factor + 0.5) / factor
end


-- export (selected) functions to Lua "math" namespace
function _M.register_to_Lua()
	for _, func in ipairs({"frac", "round", "trunc"}) do
		math[func] = _M[func]
	end
end

return _M
