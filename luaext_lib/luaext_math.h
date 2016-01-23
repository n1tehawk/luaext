// luaext_math.h

#ifndef LUAEXT_MATH_H
#define LUAEXT_MATH_H

#include "lua.h"
#include "luahelpers.h"

// "cutoff" value to detect zero or small differences
#define LUAEXT_MATH_EPSILON	1E-16

LUA_CFUNC(luaext_math_trunc);

#endif // LUAEXT_MATH_H
