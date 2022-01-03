local function openFile()
    local file = assert(io.open('./signo.txt'))
    local tbl = {}
    local arr = {}

    -- remove duplicate
    for line in file:lines() do
        local errno = line:match('^SIG[A-Z0-9]+$')
        if not errno then
            error('invalid line: ' .. line)
        end
        tbl[errno] = true;
    end

    for errno in pairs(tbl) do
        arr[#arr + 1] = errno
    end
    table.sort(arr)

    return arr
end

local function codeGen(arr)
    local fmtConstants = [[
#ifdef %s
    lauxh_pushint2tbl( L, "%s", %s );
#endif

]];
    local errnoConstants = ''

    for _, v in ipairs(arr) do
        errnoConstants = errnoConstants .. fmtConstants:format(v, v, v)
    end

    return {
        DECL = errnoConstants,
    };
end

local function inject(tbl)
    local file = io.open('./signal_tmpl.c'):read('*a')
    local replace = function(match)
        return tbl[match];
    end

    file = file:gsub('#define GEN_SIGNO_(%w+)\n', replace)
    io.open('./signal.c', 'w'):write(file)
end

inject(codeGen(openFile()))
