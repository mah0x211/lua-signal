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
#include <lauxlib.h>
#include <lualib.h>
#include "lauxhlib.h"


static int block_lua( lua_State *L )
{
    sigset_t ss;
    lua_Integer signo;
    
    sigemptyset( &ss );
    if( !lua_gettop( L ) || !lauxh_isinteger( L, 1 ) ||
        ( signo = lua_tointeger( L, 1 ) ) >= NSIG ){
        errno = EINVAL;
    }
    else if( sigaddset( &ss, (int)signo ) == 0 && 
             sigprocmask( SIG_BLOCK, &ss, NULL ) == 0 ){
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
    sigset_t ss;
    lua_Integer signo;
    
    sigemptyset( &ss );
    if( !lua_gettop( L ) || !lauxh_isinteger( L, 1 ) ||
        ( signo = lua_tointeger( L, 1 ) ) >= NSIG ){
        errno = EINVAL;
    }
    else if( sigprocmask( 0, NULL, &ss ) == 0 ){
        lua_pushboolean( L, sigismember( &ss, (int)signo ) == 1 );
        return 1;
    }
    
    // got error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
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
    sigset_t ss;
    lua_Integer signo;
    
    sigemptyset( &ss );
    if( !lua_gettop( L ) || !lauxh_isinteger( L, 1 ) ||
        ( signo = lua_tointeger( L, 1 ) ) >= NSIG ){
        errno = EINVAL;
    }
    else if( sigaddset( &ss, (int)signo ) == 0 && 
             sigprocmask( SIG_UNBLOCK, &ss, NULL ) == 0 ){
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
    lua_Integer signo;
    
    if( !lua_gettop( L ) || !lauxh_isinteger( L, 1 ) ||
        ( signo = lua_tointeger( L, 1 ) ) >= NSIG ){
        errno = EINVAL;
    }
    else if( raise( (int)signo ) == 0 ){
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
    lua_Integer signo;
    
    if( !lua_gettop( L ) || !lauxh_isinteger( L, 1 ) ||
        !lauxh_isinteger( L, 2 ) ||
        ( signo = lua_tointeger( L, 2 ) ) >= NSIG ){
        errno = EINVAL;
    }
    else
    {
        pid_t pid = (pid_t)lua_tointeger( L, 1 );
        
        if( kill( pid, (int)signo ) == 0 ){
            lua_pushboolean( L, 1 );
            return 1;
        }
    }
    
    // got error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int killpg_lua( lua_State *L )
{
    lua_Integer signo;
    
    if( !lua_gettop( L ) || !lauxh_isinteger( L, 1 ) ||
        !lauxh_isinteger( L, 2 ) ||
        ( signo = lua_tointeger( L, 2 ) ) >= NSIG ){
        errno = EINVAL;
    }
    else
    {
        pid_t pid = (pid_t)lua_tointeger( L, 1 );
        
        if( killpg( pid, (int)signo ) == 0 ){
            lua_pushboolean( L, 1 );
            return 1;
        }
    }
    
    // got error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
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
