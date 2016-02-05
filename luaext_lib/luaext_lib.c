/*
 * luaext_lib.c
 * A C module with low-level functions for luaext
 */
/// @module luaext
#include "luaext_lib.h"
#include "luaext_math.h"
#include "luaext_refs.h"
#include "luaext_string.h"
#include "luaext_table.h"

#include "lauxlib.h"
#include "luacompat.h"
#include <string.h>


/** C-style printf, an equivalent to `print(string.format(...))`.
<br>Note:
`printf(nil)` will simply output an empty line, just like `print()` would do.

@function printf
@tparam string fmt Formatting string, see `string.format`
@param ... (Any additional varargs will be passed through to string.format)
@usage
printf("answer = %d", 42)
*/
LUA_CFUNC(luaext_printf) {
	int nargs = lua_gettop(L);
	if (nargs > 0) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, STRING_FORMAT);
		lua_insert(L, 1);
		lua_call(L, nargs, 1);
		lua_getglobal(L, "print");
		lua_insert(L, 1);
		lua_call(L, 1, 0);
	} else {
		lua_getglobal(L, "print");
		lua_call(L, 0, 0);
	}
	return 0;
}

/** printf-style error() function.
`error_fmt(level, fmt, ...)` is equivalent to `error(string.format(fmt, ...), level)`

@usage
function bar(level)
	if level then
		error_fmt(level, "%s went wrong.", "Something")
	else
		error_fmt("%s went wrong.", "Something")
	end
end
function foo(level)
	bar(level)
end

foo()  -- will report error position with default level = within bar()
foo(2) -- will report error position within foo()

@function error_fmt
@tparam number level Error position (nesting level) to report, see description
of standard Lua `error()`.
<br>Note: `level` is optional, with `error_fmt(fmt, ...)` implemented as
`error_fmt(1, fmt, ...)`.
@tparam string fmt Formatting string, see `string.format`
@param ... (any additional varargs will be passed through to string.format)
*/
LUA_CFUNC(luaext_error_fmt) {
	int level = 1; // default error level (report position within current chunk)
	switch (lua_type(L, 1)) {
	case LUA_TNUMBER:
		level = luaL_checkinteger(L, 1);
		if (level < 1) level = 1; // ignore level value if it's too small
		break;
	case LUA_TSTRING: // use default level, but (re)align message on Lua stack
		lua_pushnil(L);
		lua_insert(L, 1);
		break;
	}
	luaL_checkstring(L, 2);

	int nargs = lua_gettop(L) - 2;
	// TODO: optimize for nargs == 0, by simply passing through message string ?
	lua_rawgeti(L, LUA_REGISTRYINDEX, STRING_FORMAT);
	lua_insert(L, 2); // move function before fmt string (plus any optional varargs)
	lua_call(L, nargs + 1, 1);

	lua_getglobal(L, "error");
	lua_replace(L, 1);
	lua_pushinteger(L, level + 1);
	lua_call(L, 2, 0); // error(msg, level)
	return 0; // (Note: actually this function doesn't ever return)
}

// tests for an 'empty' value at given stack index
bool luaext_isEmpty(lua_State *L, int idx) {
	LUA_CHECKIDX(L, idx);
	if (lua_isnoneornil(L, idx)) return true; // (no value at index)

	switch (lua_type(L, idx)) {
	case LUA_TSTRING:
		// a string is 'empty' if it has zero length (== "")
		return lua_rawlen(L, idx) == 0;

	case LUA_TTABLE:
		LUA_ABSIDX(L, idx);
		// we'll test if the table contains at least one element (index)
		lua_pushnil(L);
		if (lua_next(L, idx)) {
			lua_pop(L, 2);
			return false; // this is a non-empty table
		}
		return true; // empty table

	case LUA_TLIGHTUSERDATA:
	case LUA_TUSERDATA:
		// test if it's a NULL pointer
		return lua_touserdata(L, idx) == NULL;

#ifdef LUA_TCDATA
	case LUA_TCDATA:
		/* LuaJIT <cdata> is a bit tricky. A first attempt to simply compare
		 * the value to `nil` using lua_equal() FAILED.
		 * According to http://comments.gmane.org/gmane.comp.lang.lua.luajit/4488
		 * the Lua C API isn't guaranteed to fully interoperate with cdata.
		 *
		 * More or less the same applies to lua_topointer(), which won't produce
		 * meaningful results for cdata pointer or array types (no dereferencing).
		 *
		 * So let's resort to a workaround instead: we'll do a Lua tostring()
		 * conversion of the cdata, and check if that string contains "NULL".
		 */
		LUA_ABSIDX(L, idx);
		lua_getglobal(L, "tostring");
		lua_pushvalue(L, idx); // duplicate value
		lua_call(L, 1, 1);
		bool result = strstr(lua_tostring(L, -1), "NULL") != NULL;
		lua_pop(L, 1);
		return result;
#endif
	}
	return false; // consider anything else a non-empty value
}

// test (each argument) for 'empty' Lua values, returns same number of results
LUA_CFUNC(luaext_empty) {
	int argc = lua_gettop(L); // number of arguments, stack "top"
	int i = 1;
	while (i <= argc) lua_pushboolean(L, luaext_isEmpty(L, i++));
	return argc;
}

// helper function for cross_type_compare()
int cross_type_ordering(int type) {
	// (this determines precedence between different types)
	switch (type) {
	case LUA_TNIL: return 0;
	case LUA_TNUMBER: return 1;
	case LUA_TBOOLEAN: return 2;
	case LUA_TSTRING: return 3;
	case LUA_TTABLE: return 4;
	}
	return 5; // default, "other"
}
// "cross-type" comparison was inspired by https://github.com/bluebird75/luaunit/issues/29
int cross_type_compare(lua_State *L, int index1, int index2) {
	int order_1 = cross_type_ordering(lua_type(L, index1));
	int order_2 = cross_type_ordering(lua_type(L, index2));
	if (order_1 != order_2) {
		// we have different types, so we base the "comparison" on their precedence
		/* DEBUG only: print precedence
		lua_getglobal(L, "print");
		lua_pushliteral(L, "precedence:");
		lua_pushinteger(L, order_1);
		lua_pushinteger(L, order_2);
		lua_call(L, 3, 0);
		//*/
		if (order_1 < order_2) return -1;
		return 1;
	}
	if (order_1 == 1 || order_1 == 3) {
		// number or string, we'll use the standard Lua comparison
		if (lua_lessthan(L, index1, index2)) return -1;
		if (lua_equal(L, index1, index2)) return 0;
		return 1;
	}
	// Any other type(s) might not have a meaningful comparison / order.
	// We'll use a workaround instead: convert both values to their tostring()
	// representation, and compare that instead.
	LUA_ABSIDX(L, index1);
	LUA_ABSIDX(L, index2);
	lua_getglobal(L, "tostring");
	lua_pushvalue(L, index2);
	lua_pushvalue(L, -2); // duplicate tostring() function
	lua_pushvalue(L, index1);
	lua_call(L, 1, 1); // tostring(value1)
	lua_insert(L, -3);
	lua_call(L, 1, 1); // tostring(value2)
	/* DEBUG only: print both values
	lua_getglobal(L, "print");
	lua_pushvalue(L, -3);
	lua_pushvalue(L, -3);
	lua_call(L, 2, 0);
	//*/
	int result = 1;
	if (lua_lessthan(L, -2, -1))
		result = -1;
	else
		if (lua_equal(L, -2, -1)) result = 0;
	lua_pop(L, 2);
	return result;
}

// A Lua binding for cross_type_compare()
LUA_CFUNC(luaext_crossTypeCompare) {
	lua_settop(L, 2);
	lua_pushinteger(L, cross_type_compare(L, 1, 2));
	return 1;
}
// This function is intended to be used with table.sort()
LUA_CFUNC(luaext_crossTypeSort) {
	lua_settop(L, 2);
	lua_pushboolean(L, cross_type_compare(L, 1, 2) < 0);
	return 1;
}


// the list of module functions to export
static const luaL_Reg module_functions[] = {
	{"crossTypeCompare", luaext_crossTypeCompare},
	{"crossTypeSort", luaext_crossTypeSort},
	{"empty", luaext_empty},
	{"error_fmt", luaext_error_fmt},
	{"math_frac", luaext_math_frac},
	{"math_trunc", luaext_math_trunc},
	{"printf", luaext_printf},
	{"string_escape_lua_pattern", luaext_string_escape_lua_pattern},
	{"table_keyof", luaext_table_keyof},
	{"table_keys", luaext_table_keys},
	{NULL, NULL}
};

LUA_CFUNC(luaopen_luaext_lib) {
	luaext_create_refs(L); // create references for some frequently used funcs
	luaL_newlib(L, module_functions);
	return 1;
}
