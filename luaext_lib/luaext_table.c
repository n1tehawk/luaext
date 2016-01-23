#include "luaext_table.h"

#include "luaext_lib.h"
#include "luaext_refs.h"

#include "lauxlib.h"
#include "luacompat.h"

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
