rockspec_format = "3.0"
package = "signal"
version = "scm-1"
source = {
    url = "git+https://github.com/mah0x211/lua-signal.git"
}
description = {
    summary = "signal module",
    homepage = "https://github.com/mah0x211/lua-signal",
    license = "MIT/X11",
    maintainer = "Masatoshi Fukunaga"
}
dependencies = {
    "lua >= 5.1",
    "error >= 0.6.0",
}
build = {
    type = "make",
    build_variables = {
        PACKAGE         = "signal",
        CFLAGS          = "$(CFLAGS)",
        WARNINGS        = "-Wall -Wno-trigraphs -Wmissing-field-initializers -Wreturn-type -Wmissing-braces -Wparentheses -Wno-switch -Wunused-function -Wunused-label -Wunused-parameter -Wunused-variable -Wunused-value -Wuninitialized -Wunknown-pragmas -Wshadow -Wsign-compare",
        CPPFLAGS        = "-I$(LUA_INCDIR) -Ideps/lauxhlib",
        LDFLAGS         = "$(LIBFLAG)",
        LIB_EXTENSION   = "$(LIB_EXTENSION)"
    },
    install_variables = {
        PACKAGE         = "signal",
        LIBDIR          = "$(LIBDIR)",
        PREFIX          = "$(PREFIX)",
        LIB_EXTENSION   = "$(LIB_EXTENSION)"
    }
}
