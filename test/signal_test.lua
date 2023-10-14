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
        SIGNALS[#SIGNALS + 1] = v
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

function testcase.block()
    assert(signal.unblockall())
    -- test that block signal
    for _, signo in ipairs(SIGNALS) do
        assert.is_false(signal.isblock(signo))
        assert(signal.block(signo))
        assert(signal.isblock(signo))
    end

    -- test that return error if signal number is invalid
    local ok, err = signal.block(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)
end

function testcase.isblock()
    assert(signal.unblockall())
    -- test that return false if signal is not blocked
    assert.is_false(signal.isblock(signal.SIGUSR2))

    -- test that return error if signal number is invalid
    local ok, err = signal.isblock(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)
end

function testcase.blockall()
    -- test that block all signal
    assert(signal.blockall())
    for _, signo in ipairs(SIGNALS) do
        assert(signal.isblock(signo))
    end
end

function testcase.unblock()
    -- test that unblock signal
    assert(signal.blockall())
    for _, signo in ipairs(SIGNALS) do
        assert.is_true(signal.isblock(signo))
        assert(signal.unblock(signo))
        assert.is_false(signal.isblock(signo))
    end

    -- test that return error if signal number is invalid
    local ok, err = signal.unblock(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)
end

function testcase.unblockall()
    -- test that unblock all signal
    assert(signal.unblockall())
    for _, signo in ipairs(SIGNALS) do
        assert.is_false(signal.isblock(signo))
    end
end

function testcase.ignore_default()
    -- test that ignore signal
    assert(signal.ignore(signal.SIGUSR2))
    assert(signal.default(signal.SIGUSR2))

    -- test that return error if signal number is invalid
    local ok, err = signal.ignore(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)
end

function testcase.alarm()
    -- test that set alarm
    assert(signal.block(signal.SIGALRM))
    signal.alarm(1)
    local sig, err, timeout = signal.wait(1.5, signal.SIGALRM)
    assert.equal(sig, signal.SIGALRM)
    assert.is_nil(err)
    assert.is_nil(timeout)
end

function testcase.wait()
    local pid = getpid()

    -- test that wait signal
    local t = gettime()
    local sig, err, timeout = signal.wait(1.5, signal.SIGUSR2)
    t = gettime() - t
    assert.is_nil(sig)
    assert.is_nil(err)
    assert.is_true(timeout)
    assert.greater_or_equal(t, 1.5)
    assert.less(t, 1.6)

    -- test that wait signal forever
    local p = assert(fork())
    if p:is_child() then
        sleep(0.2)
        signal.kill(signal.SIGUSR2, pid)
        os.exit(0)
    end
    sig, err, timeout = signal.wait(nil, signal.SIGUSR2)
    assert.equal(sig, signal.SIGUSR2)
    assert.is_nil(err)
    assert.is_nil(timeout)

    -- test that can be wait even blocked signal
    p = assert(fork())
    if p:is_child() then
        sleep(0.2)
        signal.kill(signal.SIGUSR1, pid)
        os.exit(0)
    end
    sig, err, timeout = signal.wait(1, signal.SIGUSR2, signal.SIGUSR1)
    assert.equal(sig, signal.SIGUSR1)
    assert.is_nil(err)
    assert.is_nil(timeout)
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

local function consume_signals()
    signal.wait()
end

io.stdout:setvbuf('no')
for _, t in ipairs(testcase) do
    -- assert(signal.blockall())
    io.stdout:write(t.name .. ' ... ')
    local ok, err = xpcall(t.func, debug.traceback)
    if ok then
        print('ok')
    else
        print('failed')
        print(err)
    end
    assert(signal.unblockall())
    consume_signals()
end
print(string.rep('-', 40))
print('all tests passed')
