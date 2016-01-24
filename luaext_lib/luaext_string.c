#include "luaext_string.h"

#include "lauxlib.h"

// Escape a string so it can safely be used as a Lua pattern without triggering
// special semantics. This means prepending any "magic" character ^$()%.[]*+-?
// with a percent sign. For Lua 5.1 we also escape embedded NUL chars to %z.
LUA_CFUNC(luaext_escape_pattern) {
	if (lua_isnoneornil(L, 1)) return 0;
	luaL_checktype(L, 1, LUA_TSTRING);
	size_t len, i;
	const char *s = lua_tolstring(L, 1, &len);
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	for (i = 0; i < len; i++)
		switch (s[i]) {
#if LUA_VERSION_NUM < 502
		case '\0':
			luaL_addlstring(&b, "%z", 2);
			break;
#endif
		case '^':
		case '$':
		case '(':
		case ')':
		case '%':
		case '.':
		case '[':
		case ']':
		case '*':
		case '+':
		case '-':
		case '?':
			luaL_addchar(&b, '%'); // prefix the character with a '%'
		default:
			luaL_addchar(&b, s[i]);
		}
	luaL_pushresult(&b);
	return 1;
}
