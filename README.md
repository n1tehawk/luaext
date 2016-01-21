[![License](http://img.shields.io/badge/License-MIT-green.svg)](#license)

# luaext

Lua extensions library

Copyright (C) 2016 by Bernhard Nortmann, https://github.com/n1tehawk/luaext

*luaext* is a collection of Lua modules that add useful extensions to the
standard set of Lua functions / libraries.
A core module `luaext_lib` (containing low-level functions) is implemented
in C, while other additions are written in Lua.
*luaext* tries to combine versatility of functions with performance and ease of use.


## Documentation

(To be done... probably will be LDoc-based)


## Usage example

In default configuration, a simple `require("luaext")` should be enough to
integrate *luaext* into your project. It will create a global (module) table
`luaext`, and also add a number of functions to the Lua environment /
namespaces. Sometimes this is undesirable, so *luaext* can be configured to
avoid "namespace pollution" and provide everything under a private module table.

```lua
require("luaext")

local t = {"a", "b", [true]="foo", [false]="bar", [6]="six", nil, "c", [{}]=""}
for k, v in sortedpairs(t) do
	print(k, v, empty(v))
end
```

## Compilation and Installation

The recommended (and most portable) way to build and install from source is
`luarocks make`. For platforms where this isn't feasible (or luarocks not
available), a Makefile is provided alternatively.

The Makefile also allows some customization by setting (environment) variables,
e.g. to build against LuaJIT your could use something like<br>
`make -B LUA_INCDIR=-I/usr/local/include/luajit-2.0/ LUA_LIB=-lluajit-5.1 LUA_LIBDIR=-L/usr/local/lib`

## License

*luaext* is licensed under the terms of the
[MIT license](http://opensource.org/licenses/mit-license.php) reproduced below,
the same liberal conditions as Lua itself. This means that *luaext* is free
software and can be used for both academic and commercial purposes at absolutely
no cost.

---
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
