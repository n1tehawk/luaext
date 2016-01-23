// luaext_table.h

#ifndef LUAEXT_TABLE_H
#define LUAEXT_TABLE_H

#include "lua.h"
#include "luahelpers.h"
#include <stdbool.h>

bool luaext_table_pushkeyof(lua_State *L, int table);
int luaext_table_pushkeys(lua_State *L, int table, int sort, int filter);

LUA_CFUNC(luaext_table_keyof);
LUA_CFUNC(luaext_table_keys);

#endif // LUAEXT_TABLE_H
