/*
 * luaext_refs.c
 * provides some references for Lua functions, so C code can retrieve them easily
 */

#include "luaext_refs.h"
#include "lauxlib.h"

int STRING_FORMAT = LUA_NOREF;
int TABLE_SORT = LUA_NOREF;

// require given library name, retrieve a function, and create a reference to it
static int luaext_create_ref(lua_State *L,
		const char *libname, const char *funcname)
{
	lua_getglobal(L, "require");
	lua_pushstring(L, libname);
	lua_call(L, 1, 1);
	lua_pushstring(L, funcname);
	lua_rawget(L, -2);
	return luaL_ref(L, LUA_REGISTRYINDEX);
}

void luaext_create_refs(lua_State *L) {
	// store a reference to the string.format function
	STRING_FORMAT = luaext_create_ref(L, "string", "format");
	// store a reference to the table.sort function
	TABLE_SORT = luaext_create_ref(L, "table", "sort");
}
