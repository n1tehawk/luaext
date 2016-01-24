package = "luaext"
version = "scm-0" -- (always latest version from SCM)
source = {
	url = "", -- build straight from provided source
	dir = '.'
}
description = {
	summary = "A library of Lua extensions",
	detailed = [[
luaext is a collection of Lua modules that add useful extensions to the
standard set of Lua functions / libraries. A core module luaext_lib (containing
low-level functions) is implemented in C, while other additions are written in
Lua. luaext tries to combine versatility of functions with performance and ease
of use.
	]],
	homepage = "https://github.com/n1tehawk/luaext",
	license = "MIT",
}
dependencies = {
	"lua >= 5.1"
}
build = {
	type = "builtin",
	modules = {
		luaext_lib = {
			"luaext_lib/luaext_lib.c",
			"luaext_lib/luaext_math.c",
			"luaext_lib/luaext_refs.c",
			"luaext_lib/luaext_string.c",
			"luaext_lib/luaext_table.c",
		},
		luaext           = "luaext.lua",
		["luaext.math"]  = "luaext/math.lua",
		["luaext.table"] = "luaext/table.lua",
	}
}
