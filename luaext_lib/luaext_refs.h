// luaext_refs.h

#ifndef LUAEXT_REFS_H
#define LUAEXT_REFS_H

#include "lua.h"

extern int STRING_FORMAT;	// references the string.format function
extern int TABLE_SORT;		// references the table.sort function

void luaext_create_refs(lua_State *L);

#endif // LUAEXT_REFS_H
