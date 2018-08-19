/*
 *  Copyright (C) 2014 Masatoshi Teruya
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <lauxlib.h>
#include <lualib.h>
#include "lauxhlib.h"


static inline int signal_checksigno( lua_State *L, int idx )
{
    int signo = lauxh_checkinteger( L, idx );

    lauxh_argcheck( L, signo > 0 && signo < NSIG, idx,
                    "signo expected, got out of range" );

    return signo;
}


static int block_lua( lua_State *L )
{
    int argc = lua_gettop( L );
    int i = 1;
    sigset_t ss;

    lauxh_argcheck( L, argc > 0, 1, "signo expected, got no value" );
    sigemptyset( &ss );
    for(; i <= argc; i++ )
    {
        int signo = signal_checksigno( L, i );

        // failed to add signo
        if( sigaddset( &ss, signo ) != 0 ){
            lua_pushboolean( L, 0 );
            lua_pushstring( L, strerror( errno ) );
            return 2;
        }
    }

    if( sigprocmask( SIG_BLOCK, &ss, NULL ) == 0 ){
        lua_pushboolean( L, 1 );
        return 1;
    }

    // got error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int isblock_lua( lua_State *L )
{
    int argc = lua_gettop( L );
    int res = 0;
    int i = 1;
    sigset_t ss;

    lauxh_argcheck( L, argc > 0, 1, "signo expected, got no value" );
    sigemptyset( &ss );
    if( sigprocmask( 0, NULL, &ss ) != 0 ){
        lua_pushboolean( L, 0 );
        lua_pushstring( L, strerror( errno ) );
        return 2;
    }

    lua_pushboolean( L, 1 );
    lua_pushnil( L );
    for(; i <= argc; i++ )
    {
        int signo = signal_checksigno( L, i );
        int rc = sigismember( &ss, signo );

        switch( rc ){
            case -1:
                lua_pushboolean( L, 0 );
                lua_pushstring( L, strerror( errno ) );
                return 2;

            case 0:
                lua_pushinteger( L, signo );
                break;
        }
    }

    res = lua_gettop( L ) - argc;
    if( res > 2 ){
        lua_pushboolean( L, 0 );
        lua_replace( L, argc + 1 );
    }

    return res;
}


static int blockAll_lua( lua_State *L )
{
    sigset_t ss;

    sigfillset( &ss );
    if( sigprocmask( SIG_BLOCK, &ss, NULL ) == 0 ){
        lua_pushboolean( L, 1 );
        return 1;
    }

    // got error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int unblock_lua( lua_State *L )
{
    int argc = lua_gettop( L );
    int i = 1;
    sigset_t ss;

    lauxh_argcheck( L, argc > 0, 1, "signo expected, got no value" );
    sigemptyset( &ss );
    for(; i <= argc; i++ )
    {
        int signo = signal_checksigno( L, i );

        // failed to add signo
        if( sigaddset( &ss, signo ) != 0 ){
            lua_pushboolean( L, 0 );
            lua_pushstring( L, strerror( errno ) );
            return 2;
        }
    }

    if( sigprocmask( SIG_UNBLOCK, &ss, NULL ) == 0 ){
        lua_pushboolean( L, 1 );
        return 1;
    }

    // got error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int unblockAll_lua( lua_State *L )
{
    sigset_t ss;

    sigfillset( &ss );
    if( sigprocmask( SIG_UNBLOCK, &ss, NULL ) == 0 ){
        lua_pushboolean( L, 1 );
        return 1;
    }

    // got error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int raise_lua( lua_State *L )
{
    int signo = signal_checksigno( L, 1 );

    if( raise( signo ) == 0 ){
        lua_pushboolean( L, 1 );
        return 1;
    }

    // got error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int kill_lua( lua_State *L )
{
    int signo = signal_checksigno( L, 1 );
    pid_t pid = (pid_t)lauxh_optinteger( L, 2, getpid() );

    if( kill( pid, signo ) == 0 ){
        lua_pushboolean( L, 1 );
        return 1;
    }

    // got error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int killpg_lua( lua_State *L )
{
    int signo = signal_checksigno( L, 1 );
    pid_t pid = (pid_t)lauxh_optinteger( L, 2, getpgrp() );

    if( killpg( pid, signo ) == 0 ){
        lua_pushboolean( L, 1 );
        return 1;
    }

    // got error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int alarm_lua( lua_State *L )
{
    unsigned int sec = lauxh_checkuint32( L, 1 );

    lua_pushinteger( L, alarm( sec ) );

    return 1;
}


static int ignore_lua( lua_State *L )
{
    int signo = signal_checksigno( L, 1 );

    if( signal( signo, SIG_IGN ) != SIG_ERR ){
        lua_pushboolean( L, 1 );
        return 1;
    }

    // got error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 1;
}


LUALIB_API int luaopen_signal( lua_State *L )
{
    struct luaL_Reg method[] = {
        { "block", block_lua },
        { "blockAll", blockAll_lua },
        { "isblock", isblock_lua },
        { "unblock", unblock_lua },
        { "unblockAll", unblockAll_lua },
        { "raise", raise_lua },
        { "kill", kill_lua },
        { "killpg", killpg_lua },
        { "alarm", alarm_lua },
        { "ignore", ignore_lua },
        { NULL, NULL }
    };
    int i;

    // add methods
    lua_newtable( L );
    for( i = 0; method[i].name; i++ ){
        lauxh_pushfn2tbl( L, method[i].name, method[i].func );
    }

    // set signal constants
#define GEN_SIGNO_DECL

    return 1;
}
