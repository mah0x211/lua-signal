local assert = require('assert')
local errno = require('errno')
local getpid = require('getpid')
local sleep = require('time.sleep')
local gettime = require('time.clock').gettime
local fork = require('fork')
local signal = require('signal')

-- constants
local SIGNALS = {}
for k, v in pairs(signal) do
    if type(v) == 'number' and k ~= 'SIGKILL' and k ~= 'SIGSTOP' and
        string.find(k, '^SIG') then
        SIGNALS[k] = v
    end
end

local testcase = setmetatable({}, {
    __newindex = function(t, k, v)
        assert.is_function(v)
        assert(t[k] == nil, 'duplicate testcase: ' .. k)
        rawset(t, k, true)
        rawset(t, #t + 1, {
            name = k,
            func = v,
        })
    end,
})

function testcase.blockall()
    -- test that block all signal
    assert(signal.blockall())
    for signame, signo in ipairs(SIGNALS) do
        assert(signal.isblock(signo))
        assert(signal.isblock(signame))
    end
end

function testcase.unblockall()
    -- test that unblock all signal
    assert(signal.unblockall())
    for signame, signo in pairs(SIGNALS) do
        assert.is_false(signal.isblock(signo))
        assert.is_false(signal.isblock(signame))
    end
end

function testcase.block()
    -- test that block signal by number
    assert(signal.unblockall())
    for _, signo in pairs(SIGNALS) do
        assert.is_false(signal.isblock(signo))
        assert(signal.block(signo))
        assert(signal.isblock(signo))
    end

    -- test that block signal by name
    assert(signal.unblockall())
    for signame in pairs(SIGNALS) do
        assert.is_false(signal.isblock(signame))
        assert(signal.block(signame))
        assert(signal.isblock(signame))
    end

    -- test that return error if signal number is invalid
    local ok, err = signal.block(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)

    ok, err = signal.isblock(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)

    -- test that throws an error if signal name is invalid
    err = assert.throws(signal.block, 'HELLO')
    assert.match(err, 'bad argument #1')

    err = assert.throws(signal.isblock, 'HELLO')
    assert.match(err, 'bad argument #1')
end

function testcase.unblock()
    -- test that unblock signal by number
    assert(signal.blockall())
    for _, signo in pairs(SIGNALS) do
        assert.is_true(signal.isblock(signo))
        assert(signal.unblock(signo))
        assert.is_false(signal.isblock(signo))
    end

    -- test that unblock signal by name
    assert(signal.blockall())
    for signame in pairs(SIGNALS) do
        assert.is_true(signal.isblock(signame))
        assert(signal.unblock(signame))
        assert.is_false(signal.isblock(signame))
    end

    -- test that return error if signal number is invalid
    local ok, err = signal.unblock(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)

    -- test that throws an error if signal name is invalid
    err = assert.throws(signal.unblock, 'HELLO')
    assert.match(err, 'bad argument #1')
end

function testcase.ignore_default()
    -- test that set ignore signal by number
    assert(signal.ignore(signal.SIGUSR2))

    -- test that set default signal by number
    assert(signal.default(signal.SIGUSR2))

    -- test that set ignore signal by name
    assert(signal.ignore('SIGUSR2'))

    -- test that set default signal by name
    assert(signal.default('SIGUSR2'))

    -- test that return error if signal number is invalid
    local ok, err = signal.ignore(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)

    -- test that throws an error if signal name is invalid
    err = assert.throws(signal.ignore, 'HELLO')
    assert.match(err, 'bad argument #1')

    err = assert.throws(signal.default, 'HELLO')
    assert.match(err, 'bad argument #1')
end

function testcase.alarm()
    -- test that set alarm
    assert(signal.block('SIGALRM'))
    signal.alarm(1)
    local sig, err, timeout, signame = signal.wait(1.5, 'SIGALRM')
    assert.equal(sig, signal.SIGALRM)
    assert.is_nil(err)
    assert.is_nil(timeout)
    assert.equal(signame, 'SIGALRM')
end

function testcase.wait()
    local pid = getpid()

    -- test that wait signal
    local t = gettime()
    local sig, err, timeout, signame = signal.wait(1.5, signal.SIGUSR2)
    t = gettime() - t
    assert.is_nil(sig)
    assert.is_nil(err)
    assert.is_true(timeout)
    assert.is_nil(signame)
    assert.greater_or_equal(t, 1.5)
    assert.less(t, 1.6)

    --
    -- XXX: SIGCHLD handler does not working in macOS.
    --
    -- -- test that wait SIGCHLD signal
    -- local p = assert(fork())
    -- if p:is_child() then
    --     sleep(0.2)
    --     os.exit(0)
    -- end
    -- sig, err, timeout, signame = signal.wait(1, 'SIGCHLD')
    -- assert.equal(sig, signal.SIGCHLD)
    -- assert.is_nil(err)
    -- assert.is_nil(timeout)
    -- assert.equal(signame, 'SIGCHLD')

    -- test that wait signal forever
    local p = assert(fork())
    if p:is_child() then
        sleep(0.2)
        signal.kill(signal.SIGUSR2, pid)
        os.exit(0)
    end
    sig, err, timeout, signame = signal.wait(nil, signal.SIGUSR2)
    assert.equal(sig, signal.SIGUSR2)
    assert.is_nil(err)
    assert.is_nil(timeout)
    assert.equal(signame, 'SIGUSR2')

    -- test that return nil if no signals are given and no signals are pending
    sig, err, timeout, signame = signal.wait(0.1)
    assert.is_nil(sig)
    assert.is_nil(err)
    assert.is_nil(timeout)
    assert.is_nil(signame)

    -- test that can be wait even blocked signal
    p = assert(fork())
    if p:is_child() then
        sleep(0.2)
        signal.kill(signal.SIGUSR1, pid)
        os.exit(0)
    end
    sig, err, timeout, signame = signal.wait(1, signal.SIGUSR2, signal.SIGUSR1)
    assert.equal(sig, signal.SIGUSR1)
    assert.is_nil(err)
    assert.is_nil(timeout)
    assert.equal(signame, 'SIGUSR1')
end

function testcase.raise()
    -- test that raise signal
    assert(signal.block(signal.SIGUSR2))
    assert(signal.raise(signal.SIGUSR2))
    local sig, err = signal.wait(nil, signal.SIGUSR2)
    assert(sig == signal.SIGUSR2, err)

    -- test that return error if signal number is invalid
    local ok
    ok, err = signal.raise(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)
end

function testcase.kill()
    -- test that send signal
    assert(signal.block(signal.SIGUSR2))
    assert(signal.kill(signal.SIGUSR2))
    local sig, err = signal.wait(nil, signal.SIGUSR2)
    assert(sig == signal.SIGUSR2, err)

    -- test that return error if signal number is invalid
    local ok
    ok, err = signal.kill(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)
end

function testcase.killpg()
    print('NOTE: killpg cannot be tested in this environment')

    -- -- test that send signal
    -- assert(signal.block(signal.SIGUSR2))
    -- assert(signal.killpg(signal.SIGUSR2))
    -- local sig, err = signal.wait(nil, signal.SIGUSR2)
    -- assert(sig == signal.SIGUSR2, err)

    -- -- test that return error if signal number is invalid
    -- local ok
    -- ok, err = signal.killpg(-1)
    -- assert.is_false(ok)
    -- assert.equal(err.type, errno.EINVAL)
end

function testcase.tosigname()
    for signame, signo in pairs(SIGNALS) do
        -- test that convert signal number to signal name
        assert.equal(signal.tosigname(signo), signame)

        -- test that convert signal name to signal name
        assert.equal(signal.tosigname(signame), signame)
    end

    -- test that convert multiple signal numbers to signal names
    local name1, name2 = signal.tosigname(signal.SIGUSR1, signal.SIGUSR2)
    assert.equal(name1, 'SIGUSR1')
    assert.equal(name2, 'SIGUSR2')

    -- test that return nil if signal number is invalid
    local name = signal.tosigname(-1)
    assert.is_nil(name)

    -- test that throws an error if signal name is not string or integer
    local err = assert.throws(signal.tosigname, signal.SIGUSR1 + 0.1)
    assert.match(err, 'bad argument #1')
end

local function consume_signals()
    signal.wait()
end

io.stdout:setvbuf('no')
local errors = {}
for _, t in ipairs(testcase) do
    -- assert(signal.blockall())
    io.stdout:write(t.name .. ' ... ')
    local ok, err = xpcall(t.func, debug.traceback)
    if ok then
        print('ok')
    else
        print('failed')
        print(err)
        errors[#errors + 1] = {
            name = t.name,
            err = err,
        }
    end
    assert(signal.unblockall())
    consume_signals()
end
print(string.rep('-', 40))

if #errors == 0 then
    print('all tests passed')
    return
end

print('failed tests:\n')
for _, e in ipairs(errors) do
    print(string.format('- %q: %s', e.name, e.err))
    print(string.rep('-', 40))
end
error(string.format('#%d tests failed', #errors))
