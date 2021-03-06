local lu = require("tests.luaunit")
require("os")

local test_configurations = {
	{
		EXPORT_MODULE_AS_GLOBAL = true,
		EXPORT_GLOBAL_FUNCTIONS = true,
		USE_LUA_NAMESPACES = true,
	},
	{
		EXPORT_MODULE_AS_GLOBAL = true,
		EXPORT_GLOBAL_FUNCTIONS = true,
		USE_LUA_NAMESPACES = false,
	},
	{
		EXPORT_MODULE_AS_GLOBAL = true,
		EXPORT_GLOBAL_FUNCTIONS = false,
		USE_LUA_NAMESPACES = true,
	},
	{
		EXPORT_MODULE_AS_GLOBAL = true,
		EXPORT_GLOBAL_FUNCTIONS = false,
		USE_LUA_NAMESPACES = false,
	},
	{
		EXPORT_MODULE_AS_GLOBAL = false,
		EXPORT_GLOBAL_FUNCTIONS = true,
		USE_LUA_NAMESPACES = true,
	},
	{
		EXPORT_MODULE_AS_GLOBAL = false,
		EXPORT_GLOBAL_FUNCTIONS = true,
		USE_LUA_NAMESPACES = false,
	},
	{
		EXPORT_MODULE_AS_GLOBAL = false,
		EXPORT_GLOBAL_FUNCTIONS = false,
		USE_LUA_NAMESPACES = true,
	},
	{
		EXPORT_MODULE_AS_GLOBAL = false,
		EXPORT_GLOBAL_FUNCTIONS = false,
		USE_LUA_NAMESPACES = false,
	},
}

local printf = require("luaext_lib").printf

-- support test configuration matrix
local config = tonumber(require("os").getenv("MATRIX_CONFIG"))
if config then
	LUAEXT_CONFIG = test_configurations[config]
	if LUAEXT_CONFIG then
		print("\nUsing predefined configuration matrix:")
		for k, v in pairs(LUAEXT_CONFIG) do
			printf("\t%s = %s", k, tostring(v))
		end
	end
end

-- set up / import the various test classes
dofile("tests/test_luaext.lua")
dofile("tests/test_math.lua")
dofile("tests/test_string.lua")
dofile("tests/test_table.lua")

-- and finally run them all, returning an appropriate exit code
os.exit(lu.LuaUnit.run())
