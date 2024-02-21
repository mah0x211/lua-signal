local function openFile()
    local signames = {}
    for _, filename in ipairs {
        'darwin.txt',
        'dragonflybsd.txt',
        'freebsd.txt',
        'linux.txt',
        'netbsd.txt',
        'openbsd.txt',
    } do
        local file = assert(io.open('./var/' .. filename))
        for line in file:lines() do
            local name = line:match('^SIG[A-Z0-9]+$')
            if not name then
                error('invalid line: ' .. line)
            elseif not signames[name] then
                signames[name] = true
                signames[#signames + 1] = name
            end
        end
        file:close()
    end

    table.sort(signames)
    return signames
end

local function codeGen(signames)
    local export_signals_h = [[
/**
 * src/export_signals.h
 * this file is overwritten by gen_headers.lua at compile time.
 * ${DATE}
 */
#ifndef lua_signals_h
#define lua_signals_h

#include "config.h"

#if defined(HAVE_SIGISEMPTYSET) && !defined(_GNU_SOURCE)
# define _GNU_SOURCE
#endif

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
// lua
#include <lauxlib.h>
#include <lua_errno.h>

static const char *const SIGNAMES[] = {
${SIGNAMES},
    NULL,
};

static inline int checksigname(lua_State *L, int index)
{
    switch (luaL_checkoption(L, index, NULL, SIGNAMES)) {
    default:
        return -1;

${SIGCASES}
    }
}

#endif /* lua_signals_h */
]]

    local checkoption = [[
    case ${SIGIDX}:
#ifdef ${SIGNAME}
        return ${SIGNAME};
#else
        return -1;
#endif
]];
    local export = [[
#ifdef ${SIGNAME}
    lauxh_pushint2tbl(L, "${SIGNAME}", ${SIGNAME});
#endif]]

    local namelist = {}
    local caselist = {}
    local exports = {}
    for i, v in ipairs(signames) do
        namelist[i] = string.format('    %q', v)
        caselist[i] = checkoption:gsub('${([^}]+)}', {
            SIGIDX = i - 1,
            SIGNAME = v,
        }):format(i)
        exports[i] = export:gsub('${([^}]+)}', {
            SIGNAME = v,
        })
    end
    local names = table.concat(namelist, ',\n')
    local cases = table.concat(caselist, '\n')

    return export_signals_h:gsub('${([^}]+)}', {
        DATE = os.date('%c'),
        SIGNAMES = names,
        SIGCASES = cases,
    }), [[
/**
 * src/export_signals.h
 * this file is overwritten by gen_headers.lua at compile time.
 */
]] .. table.concat(exports, '\n\n')
end

local function output(txt, exports)
    for filename, content in pairs {
        ['./src/lua_signal.h'] = txt,
        ['./src/export_signals.h'] = exports,
    } do
        local file = assert(io.open(filename, 'w+'))
        file:write(content)
        file:close()
    end
end

output(codeGen(openFile()))
