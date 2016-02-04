-- configuration file for LDoc documentation tool
-- see https://github.com/stevedonovan/LDoc

project = "luaext"
title = "luaext reference"

full_description = [[
# luaext
## Lua extensions library
*luaext* is a collection of Lua modules that add useful extensions to the
standard set of Lua functions / libraries.
See the [GitHub project page](https://github.com/n1tehawk/luaext/).

To start using *luaext*, a simple `require("luaext")` should include the
`luaext` module and make the functions available. (This is configuration
dependent, see the module description for further details.)
]]

-- files to process
file = {
	"../luaext.lua",
	"../luaext_lib/",
}

style = "." -- (use CSS template from .ldoc)
merge = true -- (common documentation for C module and Lua code)
sort = true

output = "luaext"
dir = "../doc/" -- output to doc/ directory
format = "discount" -- markdown support
