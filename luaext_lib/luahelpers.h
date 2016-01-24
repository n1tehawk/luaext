/*
 * luahelpers.h
 *
 * Lua helpers declared as macros
 */

#ifndef LUAHELPERS_H
#define LUAHELPERS_H

// a macro for proper Lua CFunction declarations
#ifndef LUA_CFUNC
# ifdef __cdecl
#  define LUA_CFUNC(fname)	int __cdecl fname(lua_State *L)
# else
#  define LUA_CFUNC(fname)	int fname(lua_State *L)
# endif
#endif


// raise Lua error, but auto-prefix message with the function name "%s() "
// (see http://www.lua.org/manual/5.1/manual.html#luaL_error)
#define luaL_error_fname(L, ...) \
do { \
	lua_pushstring(L, __func__); \
	lua_pushfstring(L, "() " __VA_ARGS__); \
	lua_concat(L, 2); \
	lua_error(L); \
} while (0)

// protect against bad Lua stack index, zero is always invalid
#define LUA_CHECKIDX(L, idx) \
do { \
	if (idx == 0) \
		luaL_error(L, "%s() Lua state %p, error: " \
			"zero is not an acceptable stack index!", __func__, L); \
} while (0)

// relative Lua stack indices sometimes are hard to maintain,
// so convert it to an absolute one (when applicable)
#define LUA_ABSIDX(L, idx) \
do { \
	if (idx < 0) { idx += lua_gettop(L); idx++; } \
} while (0)

// combine the two macros
#define LUA_CHKABSIDX(L, idx) \
do { \
	if (idx == 0) \
		luaL_error(L, "%s() Lua state %p, error: " \
			"zero is not an acceptable stack index!", __func__, L); \
	if (idx < 0) { idx += lua_gettop(L); idx++; } \
} while (0)

#endif // LUAHELPERS_H
