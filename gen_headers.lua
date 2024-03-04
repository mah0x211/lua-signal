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

local function readall(filename)
    local file = assert(io.open(filename))
    local content = assert(file:read('*a'))
    file:close()
    return content
end

local function writeout(filename, content)
    local file = assert(io.open(filename, 'w'))
    assert(file:write(content))
    file:close()
end

local function gen_inc_checksigname_h(signames)
    local CASE = [[
    case ${SIGIDX}:
#ifdef ${SIGNAME}
        return ${SIGNAME};
#else
        return -1;
#endif
]]
    local namelist = {}
    local caselist = {}
    for i, v in ipairs(signames) do
        namelist[i] = string.format('    %q', v)
        caselist[i] = CASE:gsub('${([^}]+)}', {
            SIGIDX = i - 1,
            SIGNAME = v,
        })
    end

    local names = table.concat(namelist, ',\n')
    local cases = table.concat(caselist, '\n')
    local content = readall('./src/inc_checksigname_h'):gsub('${([^}]+)}', {
        DATE = os.date('%c'),
        SIGNAMES = names,
        SIGCASES = cases,
    })
    writeout('./src/inc_checksigname.h', content)
end

local function gen_inc_tosigname_h(signames)
    local CASE = [[
#ifdef ${SIGNAME}
    case ${SIGNAME}:
        return "${SIGNAME}";
#endif]]

    local cases = {}
    for i, name in ipairs(signames) do
        cases[i] = CASE:gsub('${([^}]+)}', {
            SIGNAME = name,
        })
    end

    local content = readall('./src/inc_tosigname_h'):gsub('${([^}]+)}', {
        DATE = os.date('%c'),
        SIGCASES = table.concat(cases, '\n'),
    })
    writeout('./src/inc_tosigname.h', content)
end

local function gen_inc_export_signals_h(signames)
    local EXPORT_SIGNAME = [[
#ifdef ${SIGNAME}
    lauxh_pushint2tbl(L, "${SIGNAME}", ${SIGNAME});
#endif]]

    local list = {}
    for i, v in ipairs(signames) do
        list[i] = EXPORT_SIGNAME:gsub('${([^}]+)}', {
            SIGNAME = v,
        })
    end

    local code = table.concat(list, '\n\n')
    local content = readall('./src/inc_export_signames_h'):gsub('${([^}]+)}', {
        DATE = os.date('%c'),
        SIGNAMES = code,
    })
    writeout('./src/inc_export_signames.h', content)
end

local function gen_lua_signal_h()
    local file = assert(io.open('./src/lua_signal.h', 'r+'))
    local content = file:read('*a'):gsub('${([^}]+)}', {
        DATE = os.date('%c'),
    })
    file:seek('set')
    file:write(content)
    file:close()
end

do
    local signames = openFile()
    gen_inc_checksigname_h(signames)
    gen_inc_tosigname_h(signames)
    gen_inc_export_signals_h(signames)
    gen_lua_signal_h()
end
