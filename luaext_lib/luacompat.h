/*
 * luacompat.h
 *
 * Helper macros for compatibility between different Lua versions
 * (This file assumes you have already included basic Lua headers, e.g. lua.h)
 */

/* compatibility with older Lua versions (<5.2) */
#if LUA_VERSION_NUM < 502

	// Substitute lua_objlen() for lua_rawlen()
	#define lua_rawlen(L, index)	lua_objlen(L, index)

	// Make use of luaL_register() to achieve same result as luaL_newlib()
	#define luaL_newlib(L, funcs) do { \
		lua_newtable(L); \
		luaL_register(L, NULL, funcs); \
	} while (0)

#endif

/* API changes for 5.2+ */
#if LUA_VERSION_NUM >= 502

	// lua_compare() has replaced lua_equal() and lua_lessthan()
	#define lua_equal(L, index1, index2)	lua_compare(L, index1, index2, LUA_OPEQ)
	#define lua_lessthan(L, index1, index2)	lua_compare(L, index1, index2, LUA_OPLT)

#endif

/* API changes for 5.3+ */
#if LUA_VERSION_NUM >= 503

	// luaL_optinteger() has replaced luaL_optint()
	#define luaL_optint(L, arg, d)	luaL_optinteger(L, arg, d)

#endif

/* auto-include luajit.h, if appropriate
 * This allows you to detect LuaJIT more easily, e.g. by #ifdef LUAJIT_VERSION
 * See: http://lua-users.org/lists/lua-l/2011-11/msg01126.html
 *
 * LuaJIT unfortunately also doesn't expose it's LUA_TCDATA definition
 * (FFI type identifier for <cdata>). We use some black magic to provide
 * a definition that's hopefully compatible (see lj_obj.h for LuaJIT 2.0).
 */
#ifdef LUA_LJDIR // (should only be present on LuaJIT)
# include "luajit.h"
# define LUA_TCDATA	(LUA_TTHREAD+2)
#endif
