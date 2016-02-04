-- configuration file for LDoc documentation tool
-- see https://github.com/stevedonovan/LDoc

project = "luaext"
title = "luaext reference"

-- files to process
file = {
	--"../LuaXML.lua",
	--"../LuaXML_lib.c"
	"../luaext_lib/",
}

style = "." -- (use CSS template from .ldoc)
merge = true -- (common documentation for C module and Lua code)
sort = true

output = "luaext"
dir = "../doc/" -- output to doc/ directory
format = "discount" -- markdown support
