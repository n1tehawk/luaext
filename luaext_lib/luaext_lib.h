// luaext_lib.h

#ifndef LUAEXT_LIB_H
#define LUAEXT_LIB_H

#include "lua.h"
//#include "lualib.h"
//#include "lauxlib.h"
#include "luahelpers.h"

#include <stdbool.h>

bool luaext_isEmpty(lua_State *L, int idx); // test for "empty" value
int cross_type_compare(lua_State *L, int index1, int index2);

LUA_CFUNC(luaext_crossTypeCompare);
LUA_CFUNC(luaext_crossTypeSort);

LUA_CFUNC(luaopen_luaext_lib);

#endif // LUAEXT_LIB_H
