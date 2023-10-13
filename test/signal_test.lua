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

local testcase = {}

function testcase.block()
    -- test that block signal
    for _, signo in pairs(SIGNALS) do
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
    -- test that return false if signal is not blocked
    assert.is_false(signal.isblock(signal.SIGINT))

    -- test that return error if signal number is invalid
    local ok, err = signal.isblock(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)
end

function testcase.blockall()
    -- test that block all signal
    assert(signal.blockall())
    for _, signo in pairs(SIGNALS) do
        assert(signal.isblock(signo))
    end
end

function testcase.unblock()
    -- test that unblock signal
    assert(signal.blockall())
    for _, signo in pairs(SIGNALS) do
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
    for _, signo in pairs(SIGNALS) do
        assert.is_false(signal.isblock(signo))
    end
end

function testcase.raise()
    assert(signal.block(signal.SIGUSR1))

    -- test that raise signal
    assert(signal.raise(signal.SIGUSR1))

    -- test that return error if signal number is invalid
    local ok, err = signal.raise(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)
end

function testcase.kill()
    -- test that send signal
    assert(signal.kill(signal.SIGUSR1))

    -- test that return error if signal number is invalid
    local ok, err = signal.kill(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)
end

function testcase.killpg()
    -- test that send signal
    -- assert(signal.killpg(signal.SIGUSR1))

    -- test that return error if signal number is invalid
    local ok, err = signal.killpg(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)
end

function testcase.ignore()
    -- test that ignore signal
    assert(signal.ignore(signal.SIGINT))

    -- test that return error if signal number is invalid
    local ok, err = signal.ignore(-1)
    assert.is_false(ok)
    assert.equal(err.type, errno.EINVAL)
end

function testcase.alarm()
    -- test that set alarm
    signal.alarm(1)
    local sig, err, timeout = signal.wait(1500, signal.SIGALRM)
    assert.equal(sig, signal.SIGALRM)
    assert.is_nil(err)
    assert.is_nil(timeout)
end

function testcase.default()
    -- test that set default action
    assert(signal.default(signal.SIGUSR1))
end

function testcase.wait()
    local pid = getpid()

    -- test that wait signal
    local t = gettime()
    local sig, err, timeout = signal.wait(1.5, signal.SIGINT)
    t = gettime() - t
    assert.is_nil(sig)
    assert.is_nil(err)
    assert.is_true(timeout)
    assert.greater_or_equal(t, 1.5)
    assert.less(t, 1.6)

    -- test that wait signal
    local p = assert(fork())
    if p:is_child() then
        sleep(0.2)
        signal.kill(signal.SIGINT, pid)
        os.exit(0)
    end
    sig, err, timeout = signal.wait(1, signal.SIGINT)
    assert.equal(sig, signal.SIGINT)
    assert.is_nil(err)
    assert.is_nil(timeout)

    -- test that can be wait even blocked signal
    signal.blockall()
    p = assert(fork())
    if p:is_child() then
        sleep(0.2)
        signal.kill(signal.SIGUSR1, pid)
        os.exit(0)
    end
    sig, err, timeout = signal.wait(1, signal.SIGINT, signal.SIGUSR1)
    assert.equal(sig, signal.SIGUSR1)
    assert.is_nil(err)
    assert.is_nil(timeout)
    -- confirm signal is blocked
    assert(signal.isblock(signal.SIGINT))
    assert(signal.isblock(signal.SIGUSR1))
    signal.unblockall()
end

local function ignoreall()
    for _, v in pairs(SIGNALS) do
        assert(signal.ignore(v))
    end
end

for k, f in pairs(testcase) do
    ignoreall()
    local ok, err = xpcall(f, debug.traceback)
    if ok then
        print(k .. ': ok')
    else
        print(k .. ': failed')
        print(err)
    end
    signal.unblockall()
end
