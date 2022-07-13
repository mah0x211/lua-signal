/**
 *  Copyright (C) 2014-2022 Masatoshi Fukunaga
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
// lua
#include <lua_errno.h>

static inline int is_valid_signo(int signo)
{
    if (signo > 0 && signo < NSIG) {
        return 1;
    }
    errno = EINVAL;
    return 0;
}

static int block_lua(lua_State *L)
{
    int argc = lua_gettop(L);
    int i    = 1;
    sigset_t ss;

    lauxh_argcheck(L, argc > 0, 1, "signo expected, got no value");
    sigemptyset(&ss);
    for (; i <= argc; i++) {
        int signo = lauxh_checkinteger(L, i);

        // failed to add signo
        if (!is_valid_signo(signo) || sigaddset(&ss, signo) != 0) {
            lua_pushboolean(L, 0);
            lua_errno_new(L, errno, "block.sigaddset");
            return 2;
        }
    }

    // block signals
    if (sigprocmask(SIG_BLOCK, &ss, NULL) == 0) {
        lua_pushboolean(L, 1);
        return 1;
    }
    // got error
    lua_pushboolean(L, 0);
    lua_errno_new(L, errno, "block.sigprocmask");
    return 2;
}

static int isblock_lua(lua_State *L)
{
    int argc = lua_gettop(L);
    int nsig = argc;
    int i    = 1;
    sigset_t ss;

    lauxh_argcheck(L, argc > 0, 1, "signo expected, got no value");

    // get current blocked signals
    sigemptyset(&ss);
    if (sigprocmask(0, NULL, &ss) != 0) {
        lua_pushboolean(L, 0);
        lua_errno_new(L, errno, "isblock.sigprocmask");
        return 2;
    }

    while (i <= nsig) {
        int signo = lauxh_checkinteger(L, i);
        int rc    = 0;

        if (!is_valid_signo(signo)) {
            lua_pushboolean(L, 0);
            lua_errno_new(L, errno, "isblock.sigismember");
            return 2;
        }

        rc = sigismember(&ss, signo);
        switch (rc) {
        case -1:
            lua_pushboolean(L, 0);
            lua_errno_new(L, errno, "isblock.sigismember");
            return 2;

        case 1:
            // remove blocked signal
            lua_remove(L, i);
            nsig--;
            break;

        default:
            // skip unblocked signal
            i++;
        }
    }

    lua_pushboolean(L, nsig < argc);
    if (nsig) {
        lua_insert(L, 1);
        lua_pushnil(L);
        lua_insert(L, 2);
    }

    return lua_gettop(L);
}

static int blockall_lua(lua_State *L)
{
    sigset_t ss;

    // block all signals
    sigfillset(&ss);
    if (sigprocmask(SIG_BLOCK, &ss, NULL) == 0) {
        lua_pushboolean(L, 1);
        return 1;
    }
    // got error
    lua_pushboolean(L, 0);
    lua_errno_new(L, errno, "blockall.sigprocmask");
    return 2;
}

static int unblock_lua(lua_State *L)
{
    int argc = lua_gettop(L);
    int i    = 1;
    sigset_t ss;

    lauxh_argcheck(L, argc > 0, 1, "signo expected, got no value");
    sigemptyset(&ss);
    for (; i <= argc; i++) {
        int signo = lauxh_checkinteger(L, i);

        // failed to add signo
        if (!is_valid_signo(signo) || sigaddset(&ss, signo) != 0) {
            lua_pushboolean(L, 0);
            lua_errno_new(L, errno, "unblock.sigaddset");
            return 2;
        }
    }

    // unblock signals
    if (sigprocmask(SIG_UNBLOCK, &ss, NULL) == 0) {
        lua_pushboolean(L, 1);
        return 1;
    }
    // got error
    lua_pushboolean(L, 0);
    lua_errno_new(L, errno, "unblock.sigprocmask");
    return 2;
}

static int unblockall_lua(lua_State *L)
{
    sigset_t ss;

    // unblock all signals
    sigfillset(&ss);
    if (sigprocmask(SIG_UNBLOCK, &ss, NULL) == 0) {
        lua_pushboolean(L, 1);
        return 1;
    }
    // got error
    lua_pushboolean(L, 0);
    lua_errno_new(L, errno, "unblockall.sigprocmask");
    return 2;
}

static int raise_lua(lua_State *L)
{
    int signo = lauxh_checkinteger(L, 1);

    if (is_valid_signo(signo) && raise(signo) == 0) {
        lua_pushboolean(L, 1);
        return 1;
    }
    // got error
    lua_pushboolean(L, 0);
    lua_errno_new(L, errno, "raise");
    return 2;
}

static int kill_lua(lua_State *L)
{
    int signo = lauxh_checkinteger(L, 1);
    pid_t pid = (pid_t)lauxh_optinteger(L, 2, getpid());

    if (is_valid_signo(signo) && kill(pid, signo) == 0) {
        lua_pushboolean(L, 1);
        return 1;
    }
    // got error
    lua_pushboolean(L, 0);
    lua_errno_new(L, errno, "kill");
    return 2;
}

static int killpg_lua(lua_State *L)
{
    int signo = lauxh_checkinteger(L, 1);
    pid_t pid = (pid_t)lauxh_optinteger(L, 2, getpgrp());

    if (is_valid_signo(signo) && killpg(pid, signo) == 0) {
        lua_pushboolean(L, 1);
        return 1;
    }
    // got error
    lua_pushboolean(L, 0);
    lua_errno_new(L, errno, "killpg");
    return 2;
}

static int alarm_lua(lua_State *L)
{
    unsigned int sec = lauxh_checkuint32(L, 1);
    lua_pushinteger(L, alarm(sec));
    return 1;
}

static int ignore_lua(lua_State *L)
{
    int signo = lauxh_checkinteger(L, 1);

    if (is_valid_signo(signo) && signal(signo, SIG_IGN) != SIG_ERR) {
        lua_pushboolean(L, 1);
        return 1;
    }
    // got error
    lua_pushboolean(L, 0);
    lua_errno_new(L, errno, "ignore.signal");
    return 2;
}

static int default_lua(lua_State *L)
{
    int signo = lauxh_checkinteger(L, 1);

    if (is_valid_signo(signo) && signal(signo, SIG_DFL) != SIG_ERR) {
        lua_pushboolean(L, 1);
        return 1;
    }
    // got error
    lua_pushboolean(L, 0);
    lua_errno_new(L, errno, "default.signal");
    return 2;
}

LUALIB_API int luaopen_signal(lua_State *L)
{
    struct luaL_Reg method[] = {
        {"block",      block_lua     },
        {"blockAll",   blockall_lua  },
        {"blockall",   blockall_lua  },
        {"isblock",    isblock_lua   },
        {"unblock",    unblock_lua   },
        {"unblockAll", unblockall_lua},
        {"unblockall", unblockall_lua},
        {"raise",      raise_lua     },
        {"kill",       kill_lua      },
        {"killpg",     killpg_lua    },
        {"alarm",      alarm_lua     },
        {"ignore",     ignore_lua    },
        {"default",    default_lua   },
        {NULL,         NULL          }
    };

    lua_errno_loadlib(L);

    // add methods
    lua_newtable(L);
    for (struct luaL_Reg *ptr = method; ptr->name; ptr++) {
        lauxh_pushfn2tbl(L, ptr->name, ptr->func);
    }

    // set signal constants
#define GEN_SIGNO_DECL

    return 1;
}
