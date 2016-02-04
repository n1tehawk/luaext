/// @module luaext.math
#include "luaext_math.h"

#include "lauxlib.h"
#include <math.h>

/** return the integral part of a number (rounded towards zero).
See also: `math.modf()`
@function trunc
@tparam number n
@usage
assert(luaext.math.trunc(math.pi) == 3)
assert(luaext.math.trunc(-1.75) == -1)
*/
LUA_CFUNC(luaext_math_trunc) {
	lua_pushnumber(L, trunc(luaL_checknumber(L, 1)));
	return 1;
}
