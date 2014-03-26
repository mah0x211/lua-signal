package = "signal"
version = "scm-1"
source = {
    url = "git://github.com/mah0x211/lua-signal.git"
}
description = {
    summary = "signal module",
    homepage = "https://github.com/mah0x211/lua-signal",
    license = "MIT/X11",
    maintainer = "Masatoshi Teruya"
}
dependencies = {
    "lua >= 5.1"
}
build = {
    type = "builtin",
    modules = {
        signal = {
            sources = { "signal.c" },
        }
    }
}

