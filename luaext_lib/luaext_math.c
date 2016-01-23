#include "luaext_math.h"

#include "lauxlib.h"
#include <math.h>

// return the integral part of a number (rounded towards zero)
LUA_CFUNC(luaext_math_trunc) {
	lua_pushnumber(L, trunc(luaL_checknumber(L, 1)));
	return 1;
}
