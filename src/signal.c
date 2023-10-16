/**
 *  Copyright (C) 2014-present Masatoshi Fukunaga
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

#define NSEC 1000000000ULL

#if defined(HAVE_SIGWAITINFO) && defined(HAVE_SIGTIMEDWAIT)

static int wait_signals_lua(lua_State *L, sigset_t *ss, lua_Number sec)
{
    int signo = -1;
    sigset_t old_ss;

    errno = 0;
    if (sigprocmask(SIG_BLOCK, ss, &old_ss) != 0) {
        // got error
        lua_pushnil(L);
        lua_errno_new(L, errno, "wait.sigprocmask");
        return 2;
    }

    if (sec == 0) {
        // wait forever
        signo = sigwaitinfo(ss, NULL);
    } else {
        // wait for a specified time or until interrupted by a signal
        const struct timespec ts = {.tv_sec  = sec,
                                    .tv_nsec = (sec - (uintmax_t)sec) * NSEC};
        signo                    = sigtimedwait(ss, NULL, &ts);
    }
    // revert to old signal mask
    sigprocmask(SIG_SETMASK, &old_ss, NULL);

    if (signo != -1) {
        lua_pushinteger(L, signo);
        return 1;
    } else if (errno == EAGAIN) {
        // timeout
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushboolean(L, 1);
        return 3;
    }
    // got error
    lua_pushnil(L);
    lua_errno_new(L, errno, "wait");
    return 2;
}

#else

# include <pthread.h>
# include <sys/time.h>

typedef struct {
    int cancel;
    int sig;
    int err;
    sigset_t *sigset;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} sigwait_data_t;

static void *sigwait_in_thread(void *arg)
{
    sigwait_data_t *data = (sigwait_data_t *)arg;
    int sig              = 0;
    int rc               = sigwait(data->sigset, &sig);

    pthread_mutex_lock(&data->mutex);
    if (!data->cancel) {
        if (rc != 0) {
            data->err = errno;
        } else {
            data->sig = sig;
        }
        pthread_cond_signal(&data->cond);
    }
    pthread_mutex_unlock(&data->mutex);

    return NULL;
}

static struct timespec *get_timeout(pthread_cond_t *cond, struct timespec *ts,
                                    lua_Number deadline)
{
    uintmax_t sec  = (uintmax_t)deadline;
    uintmax_t nsec = (deadline - sec) * NSEC;

    // use monotonic clock
# if defined(HAVE_PTHREAD_CONDATTR_SETCLOCK)
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(cond, &attr);
    pthread_condattr_destroy(&attr);
    clock_gettime(CLOCK_MONOTONIC, ts);
# else
    (void)cond;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ts->tv_sec  = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;
# endif
    ts->tv_sec += sec;
    ts->tv_nsec += nsec;
    // adjust deadline if nsec is overflowed to sec
    if ((uintmax_t)ts->tv_nsec >= NSEC) {
        ts->tv_sec += 1;
        ts->tv_nsec -= NSEC;
    }
    return ts;
}

static int wait_signals_lua(lua_State *L, sigset_t *ss, lua_Number sec)
{
    sigwait_data_t data = {
        .cancel = 0,
        .sig    = -1,
        .err    = 0,
        .sigset = ss,
        .cond   = PTHREAD_COND_INITIALIZER,
        .mutex  = PTHREAD_MUTEX_INITIALIZER,
    };
    int sig            = 0;
    int err            = 0;
    int rc             = 0;
    struct timespec ts = {0};
    pthread_t th;
    sigset_t old_ss;

    errno = 0;
    if (pthread_sigmask(SIG_BLOCK, ss, &old_ss) != 0) {
        // got error
        lua_pushnil(L);
        lua_errno_new(L, errno, "wait.pthread_sigmask");
        return 2;
    }

    // wait for a specified time or until interrupted by a signal
    pthread_mutex_lock(&data.mutex);
    err = pthread_create(&th, NULL, sigwait_in_thread, &data);
    if (err) {
        printf("failed to create thread: %s\n", strerror(err));
        // revert to old signal mask
        pthread_sigmask(SIG_SETMASK, &old_ss, NULL);
        pthread_mutex_unlock(&data.mutex);
        lua_pushnil(L);
        lua_errno_new(L, err, "wait.pthread_create");
        return 2;
    }

    rc = (sec == 0) ? pthread_cond_wait(&data.cond, &data.mutex) :
                      pthread_cond_timedwait(&data.cond, &data.mutex,
                                             get_timeout(&data.cond, &ts, sec));
    // revert to old signal mask
    pthread_sigmask(SIG_SETMASK, &old_ss, NULL);

    sig         = data.sig;
    err         = data.err;
    data.cancel = 1;
    if (rc == ETIMEDOUT) {
        pthread_cancel(th);
        pthread_mutex_unlock(&data.mutex);
        pthread_join(th, NULL);
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushboolean(L, 1);
        return 3;
    }
    pthread_mutex_unlock(&data.mutex);
    pthread_join(th, NULL);

    if (err) {
        lua_pushnil(L);
        lua_errno_new(L, err, "wait");
        return 2;
    }
    lua_pushinteger(L, sig);
    return 1;
}

#endif

#if !defined(HAVE_SIGISEMPTYSET)

static inline int sigisemptyset(const sigset_t *ss)
{
    int nsig = 0;
    for (int i = 0; i < NSIG; i++) {
        nsig += sigismember(ss, i) == 1;
    }
    return nsig == 0;
}

#endif

static int wait_lua(lua_State *L)
{
    int argc       = lua_gettop(L);
    lua_Number sec = lauxh_optnumber(L, 1, 0);
    sigset_t ss;

    lauxh_argcheck(L, sec >= 0, 1, "unsigned number expected, got %f", sec);
    // register signal actions
    sigemptyset(&ss);
    if (argc > 1) {
        for (int i = 2; i <= argc; i++) {
            int sig = 0;
            if (lua_isnoneornil(L, i)) {
                // ignore nil value
                continue;
            }

            sig = lauxh_checkinteger(L, i);
            if (sig < 0 || sig >= NSIG) {
                errno = EINVAL;
            } else if (sigaddset(&ss, sig) == 0) {
                continue;
            }
            lua_pushnil(L);
            lua_errno_new(L, errno, "wait.sigaddset");
            return 2;
        }
    } else if (sigpending(&ss) != 0) {
        // got error
        lua_pushnil(L);
        lua_errno_new(L, errno, "wait.sigpending");
        return 2;
    }

    if (sigisemptyset(&ss) == 1) {
        // do nothing if no signals are specified or no pending signals
        return 0;
    }

    return wait_signals_lua(L, &ss, sec);
}

LUALIB_API int luaopen_signal(lua_State *L)
{
    struct luaL_Reg funcs[] = {
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
        {"wait",       wait_lua      },
        {NULL,         NULL          }
    };

    lua_errno_loadlib(L);

    // add methods
    lua_newtable(L);
    for (struct luaL_Reg *ptr = funcs; ptr->name; ptr++) {
        lauxh_pushfn2tbl(L, ptr->name, ptr->func);
    }

// set signal constants
#include "gen_signals.h"

    return 1;
}
