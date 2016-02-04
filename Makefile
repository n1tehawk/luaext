#
# luaext Makefile
#

LUA ?= lua
CFLAGS += -Wall -Werror $(LUA_INCDIR)

LUAEXT_OBJS =  luaext_lib/luaext_lib.o
LUAEXT_OBJS += luaext_lib/luaext_math.o
LUAEXT_OBJS += luaext_lib/luaext_refs.o
LUAEXT_OBJS += luaext_lib/luaext_string.o
LUAEXT_OBJS += luaext_lib/luaext_table.o

# platform specific rules:
ARCH = $(shell uname -s)
ifeq ($(ARCH),Linux)
  CFLAGS     += -fPIC
  LFLAGS     =  -fPIC -shared
  LUA_LIB    ?= -llua
  LIBS       = $(LUA_LIBDIR) $(LUA_LIB) -ldl
  LUAEXT_LIB = luaext_lib.so

else
  ifeq ($(ARCH),Darwin) # MacOSX
    LFLAGS     = -bundle
    LUA_LIBDIR ?= -L/usr/local/lib
    LUA_LIB    ?= -llua
    LIBS       = $(LUA_LIBDIR) $(LUA_LIB)
    LUAEXT_LIB = luaext_lib.so

  else  # Windows, MinGW
    LFLAGS     =  -shared
    LUA_LIB    ?= -llua51
    LIBS       = $(LUA_LIBDIR) $(LUA_LIB) -mconsole -s
    LUAEXT_LIB = luaext_lib.dll

  endif
endif

$(LUAEXT_LIB): $(LUAEXT_OBJS)
	$(CC) -o $@ $(LFLAGS) $^ $(LIBS)

test: $(LUAEXT_LIB)
	@# make sure Lua interpreter is available and print its version
	$(LUA) -v
	# test default configuration
	$(LUA) tests/test.lua -v

matrix: $(LUAEXT_LIB)
	@# make sure Lua interpreter is available and print its version
	$(LUA) -v
	# step through configuration matrix (see test.lua), "0" = no/default config
	@MATRIX_CONFIG=0 $(LUA) tests/test.lua -v
	@MATRIX_CONFIG=1 $(LUA) tests/test.lua -v
	@MATRIX_CONFIG=2 $(LUA) tests/test.lua -v
	@MATRIX_CONFIG=3 $(LUA) tests/test.lua -v
	@MATRIX_CONFIG=4 $(LUA) tests/test.lua -v
	@MATRIX_CONFIG=5 $(LUA) tests/test.lua -v
	@MATRIX_CONFIG=6 $(LUA) tests/test.lua -v
	@MATRIX_CONFIG=7 $(LUA) tests/test.lua -v
	@MATRIX_CONFIG=8 $(LUA) tests/test.lua -v

clean:
	rm -f $(LUAEXT_LIB) luaext_lib/*.o

luajit:
	make -B LUA_INCDIR=-I/usr/local/include/luajit-2.0/ LUA_LIB=-lluajit-5.1 LUA_LIBDIR=-L/usr/local/lib

rockspec: # build from rockspec
	luarocks make luaext-scm-0.rockspec

# generate documentation (requires LDoc)
.PHONY: doc
doc:
	ldoc -c .ldoc/config.ld .
