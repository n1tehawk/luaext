/*
 * luaext_lib.c
 * A C module with low-level functions for luaext
 */
#include "luaext_lib.h"

#include "lauxlib.h"
#include "luacompat.h"
#include <string.h>

static int STRING_FORMAT = LUA_NOREF;	// references the string.format function
static int TABLE_SORT = LUA_NOREF;		// references the table.sort function

/* General / "global" functions */

// C-style printf, an equivalent to print(string.format(...))
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

// printf-style error:
// error_fmt(level, fmt, ...) is equivalent to error(string.format(fmt, ...), level)
// level is optional, with error_fmt(fmt, ...) implemented as error_fmt(1, fmt, ...)
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

/* Math */

/* Strings */

/* Tables */

// Try to find value in a table, and return its key (table index) or `nil` if
// not found. Parameter "table" is the stack index of the table to search, and
// the value to look for is expected at the top of the stack. Will push the
// first key k that satisfies (t[k] == value), or `nil` if there's no match.
// (The return value also indicates if there was a match.)
//
// Note: This function can be 'expensive', in that it will cause a sequential
// access to many (or even all) table keys with a value that has a disadvantageous
// position or isn't contained in the table. Each step in the search iteration
// also requires a comparison, which might in turn invoke metamethods ("__eq").
bool luaext_table_pushkeyof(lua_State *L, int table) {
	LUA_ABSIDX(L, table);
	luaL_checktype(L, table, LUA_TTABLE);

	// we'll do a quick exit if the value is `nil`
	// (which is impossible to match as a table element)
	if (lua_isnil(L, -1)) return false;

	lua_pushnil(L);
	while (lua_next(L, table)) {
		/* DEBUG only
		luautils_getfunction(L, NULL, "tostring", true);
		lua_pushvalue(L, -2); // duplicate value
		lua_call(L, 1, 1); // string conversion
		debug("%s(%d) iterating %s '%s'", __func__, table,
				luaL_typename(L, -2), lua_tostring(L, -1));
		lua_pop(L, 1); // (drop value string)
		//*/
#ifdef LUA_TCDATA
		// Note: DON't use lua_equal() when having to deal with LUA_TCDATA!
		if (cross_type_compare(L, -1, -3) == 0) {
#else
		if (lua_equal(L, -1, -3)) {
#endif
			// we have a match. pop value, replace initial value with key
			lua_pop(L, 1);
			lua_replace(L, -2);
			return true;
		}
		lua_pop(L, 1); // (discard value)
	}

	// no matching key, pop value and push `nil`
	lua_pop(L, 1);
	lua_pushnil(L);
	return false;
}

// find key for a value (arg #2) within a table (arg #1)
LUA_CFUNC(luaext_table_keyof) {
	luaext_table_pushkeyof(L, 1);
	return 1;
}

// Push a new sequential table ("array") with all the keys from a given table.
// Parameter "table" is the stack index of the table to process.
//
// "sort" can optionally specify the stack index of a sorting function. If set
// set (i.e. != 0), then table.sort(result, sort_func) will be called before
// returning the result, meaning you receive a table of keys sorted according
// to the given function. Alternatively, if the "sort" stack index doesn't point
// to a function value, we'll check the boolean Lua evaluation of it. If that
// results in `true`, use a standard table.sort(result).
//
// "filter" is also optional, and specifies the stack index of a filter function
// to be called for each key. If set, the filter is expected to receive a single
// parameter (the current key), and to return a boolean result indicating
// whether to include that particular key.
//
// This function pushes the resulting array (table) as the topmost value onto
// the Lua stack, and returns the number of keys (elements in the array).
int luaext_table_pushkeys(lua_State *L, int table, int sort, int filter) {
	LUA_ABSIDX(L, table);
	luaL_checktype(L, table, LUA_TTABLE);
	if (filter) {
		LUA_ABSIDX(L, filter);
		luaL_checktype(L, filter, LUA_TFUNCTION);
	}
	if (sort) {
		// we'll either keep the (absolute) sort index in case it references a
		// function, make it -1 to indicate a boolean `true` (standard sort
		// request), or just set it to 0 to avoid sorting
		LUA_ABSIDX(L, sort);
		if (lua_type(L, sort) != LUA_TFUNCTION)
			sort = lua_toboolean(L, sort) ? -1 : 0;
	}
	lua_newtable(L);
	int result = lua_gettop(L); // stack index of result table
	int count = 0; // key count, = index for result table
	bool use_key = true;

	// iterate the source table
	lua_pushnil(L);
	while (lua_next(L, table)) {
		lua_pop(L, 1); // ignore (and pop) the value
		if (filter) {
			lua_pushvalue(L, filter); // duplicate filter function
			lua_pushvalue(L, -2); // duplicate key
			lua_call(L, 1, 1);
			use_key = lua_toboolean(L, -1);
			lua_pop(L, 1); // remove result (realign Lua stack)
		}
		if (use_key) {
			lua_pushvalue(L, -1); // duplicate the key
			lua_rawseti(L, result, ++count); // and store it to the result table
		}
	}
	if (sort) {
		//printf("%s(): sort = %d\n", __func__, sort);
		lua_rawgeti(L, LUA_REGISTRYINDEX, TABLE_SORT);
		lua_pushvalue(L, -2); // duplicate the result (table parameter)
		if (sort > 0) {
			// we have a sort function, duplicate it - then call sort()
			lua_pushvalue(L, sort);
			lua_call(L, 2, 0);
		}
		else lua_call(L, 1, 0); // standard table.sort() with default comparison
	}
	// result table should (still) be topmost on stack
	return count;
}

// (private) helper function for table_keys_C, to gracefully handle varargs
static LUA_CFUNC(luaext_table_keys_helper) {
	// Upon entry, the Lua stack will contain the current key that the filter
	// is to examine, and the upvalues will have the actual function (mandatory)
	// plus any additional parameters (varargs, optional).

	// so let's get our filter function first, and place it _before_ the key
	lua_pushvalue(L, lua_upvalueindex(1)); // (get filter from upvalue #1)
	lua_insert(L, -2);
	// now add any (optional) varargs, keeping track of their count
	int varargs = 0, upvalue;
	while (true) {
		upvalue = lua_upvalueindex(2 + varargs);
		if (lua_isnone(L, upvalue)) break;
		lua_pushvalue(L, upvalue); // push upvalue to stack
		varargs++;
		/* DEBUG only
		lua_getglobal(L, "print");
		lua_pushinteger(L, varargs);
		lua_pushvalue(L, -3);
		lua_call(L, 2, 0);
		//*/
	}
	//int varargs = luautils_xunpack(L, lua_upvalueindex(2), 1, 0);
	// and finally call the filter function, passing through its return value
	lua_call(L, varargs + 1, 1);
	return 1;
}

// get the keys of a table: luaext_table_keys(t, sort, filter, ...)
LUA_CFUNC(luaext_table_keys) {
	int nargs = lua_gettop(L), count;
	if (nargs >= 3) {
		// we expect arg #3 to be a (filter) function.
		luaL_checktype(L, 3, LUA_TFUNCTION);
		// create a C closure that provides the filter plus any varargs
		//printf("%s(): %d upvalues\n", __func__, nargs - 2);
		lua_pushcclosure(L, luaext_table_keys_helper, nargs - 2); // sets (and pops) upvalues
		count = luaext_table_pushkeys(L, 1, 2, 3);
	}
	else
		count = luaext_table_pushkeys(L, 1, 2, 0); // arg #3 == 0 -> no filter function
	lua_pushinteger(L, count);
	return 2;
}


/* (luaext_lib) module */

// the list of module functions to export
static const luaL_Reg module_functions[] = {
	{"crossTypeCompare", luaext_crossTypeCompare},
	{"crossTypeSort", luaext_crossTypeSort},
	{"empty", luaext_empty},
	{"error_fmt", luaext_error_fmt},
	{"printf", luaext_printf},
	{"table_keyof", luaext_table_keyof},
	{"table_keys", luaext_table_keys},
	{NULL, NULL}
};

LUA_CFUNC(luaopen_luaext_lib) {
	// retrieve the string.format function and store a reference to it
	lua_getglobal(L, "require");
	lua_pushliteral(L, "string");
	lua_call(L, 1, 1);
	lua_pushliteral(L, "format");
	lua_rawget(L, -2);
	STRING_FORMAT = luaL_ref(L, LUA_REGISTRYINDEX);
	// retrieve the table.sort function and store a reference to it
	lua_getglobal(L, "require");
	lua_pushliteral(L, "table");
	lua_call(L, 1, 1);
	lua_pushliteral(L, "sort");
	lua_rawget(L, -2);
	TABLE_SORT = luaL_ref(L, LUA_REGISTRYINDEX);

	luaL_newlib(L, module_functions);
	return 1;
}
