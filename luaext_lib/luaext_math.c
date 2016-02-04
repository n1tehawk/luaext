/// @module luaext.math
#include "luaext_math.h"

#include "lauxlib.h"
#include <math.h>

/** return the fractional part of a number (preserving the sign).
See also: `trunc`, `math.modf()`

@usage
assert(luaext.math.frac(1.98) == 0.98)
assert(luaext.math.trunc(-1.75) == -0.75)
assert(luaext.math.frac(1234) == 0)

@function frac
@tparam number n
*/
LUA_CFUNC(luaext_math_frac) {
	register double d = luaL_checknumber(L, 1);
	lua_pushnumber(L, d - trunc(d));
	return 1;
}

/** return the integral part of a number (rounded towards zero).
See also: `frac`, `math.modf()`

@usage
assert(luaext.math.trunc(math.pi) == 3)
assert(luaext.math.trunc(-1.75) == -1)

@function trunc
@tparam number n
*/
LUA_CFUNC(luaext_math_trunc) {
	lua_pushnumber(L, trunc(luaL_checknumber(L, 1)));
	return 1;
}
