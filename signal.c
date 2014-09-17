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

#define lstate_num2tbl(L,k,v) do{ \
    lua_pushstring(L,k); \
    lua_pushnumber(L,v); \
    lua_rawset(L,-3); \
}while(0)

static int block_lua( lua_State *L )
{
    sigset_t ss;
    lua_Integer signo;
    
    sigemptyset( &ss );
    if( !lua_gettop( L ) || !lua_isnumber( L, 1 ) || 
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
    if( !lua_gettop( L ) || !lua_isnumber( L, 1 ) || 
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
    if( !lua_gettop( L ) || !lua_isnumber( L, 1 ) || 
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
    
    if( !lua_gettop( L ) || !lua_isnumber( L, 1 ) || 
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
    
    if( !lua_gettop( L ) || !lua_isnumber( L, 1 ) || !lua_isnumber( L, 2 ) || 
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

// to uppercase
static void str2upper( char *str )
{
    unsigned char *ptr = (unsigned char*)str;
    
NEXT_CHR:
    switch( *ptr ){
        case 0:
        break;
        // a .. z
        case 0x61 ... 0x7a:
            // 0x20 = SP
            *ptr -= 0x20;
        default:
            ptr++;
            goto NEXT_CHR;
	}
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
        { NULL, NULL }
    };
    int i;
    char buf[255];
    size_t len;
    
    // add methods
    lua_newtable( L );
    for( i = 0; method[i].name; i++ ){
        lua_pushstring( L, method[i].name );
        lua_pushcfunction( L, method[i].func );
        lua_rawset( L, -3 );
    }

    // add signal constants
#ifdef SIGABRT
    lstate_num2tbl( L, "ABRT", SIGABRT );
#endif
#ifdef SIGALRM
    lstate_num2tbl( L, "ALRM", SIGALRM );
#endif
#ifdef SIGBUS
    lstate_num2tbl( L, "BUS", SIGBUS );
#endif
#ifdef SIGCHLD
    lstate_num2tbl( L, "CHLD", SIGCHLD );
#endif
#ifdef SIGCLD
    lstate_num2tbl( L, "CLD", SIGCLD );
#endif
#ifdef SIGCONT
    lstate_num2tbl( L, "CONT", SIGCONT );
#endif
#ifdef SIGEMT
    lstate_num2tbl( L, "EMT", SIGEMT );
#endif
#ifdef SIGFPE
    lstate_num2tbl( L, "FPE", SIGFPE );
#endif
#ifdef SIGHUP
    lstate_num2tbl( L, "HUP", SIGHUP );
#endif
#ifdef SIGILL
    lstate_num2tbl( L, "ILL", SIGILL );
#endif
#ifdef SIGINFO
    lstate_num2tbl( L, "INFO", SIGINFO );
#endif
#ifdef SIGINT
    lstate_num2tbl( L, "INT", SIGINT );
#endif
#ifdef SIGIO
    lstate_num2tbl( L, "IO", SIGIO );
#endif
#ifdef SIGIOT
    lstate_num2tbl( L, "IOT", SIGIOT );
#endif
#ifdef SIGKILL
    lstate_num2tbl( L, "KILL", SIGKILL );
#endif
#ifdef SIGPIPE
    lstate_num2tbl( L, "PIPE", SIGPIPE );
#endif
#ifdef SIGPOLL
    lstate_num2tbl( L, "POLL", SIGPOLL );
#endif
#ifdef SIGPROF
    lstate_num2tbl( L, "PROF", SIGPROF );
#endif
#ifdef SIGPWR
    lstate_num2tbl( L, "PWR", SIGPWR );
#endif
#ifdef SIGQUIT
    lstate_num2tbl( L, "QUIT", SIGQUIT );
#endif
#ifdef SIGSEGV
    lstate_num2tbl( L, "SEGV", SIGSEGV );
#endif
#ifdef SIGSTKFLT
    lstate_num2tbl( L, "STKFLT", SIGSTKFLT );
#endif
#ifdef SIGSTOP
    lstate_num2tbl( L, "STOP", SIGSTOP );
#endif
#ifdef SIGSYS
    lstate_num2tbl( L, "SYS", SIGSYS );
#endif
#ifdef SIGTERM
    lstate_num2tbl( L, "TERM", SIGTERM );
#endif
#ifdef SIGTRAP
    lstate_num2tbl( L, "TRAP", SIGTRAP );
#endif
#ifdef SIGTSTP
    lstate_num2tbl( L, "TSTP", SIGTSTP );
#endif
#ifdef SIGTTIN
    lstate_num2tbl( L, "TTIN", SIGTTIN );
#endif
#ifdef SIGTTOU
    lstate_num2tbl( L, "TTOU", SIGTTOU );
#endif
#ifdef SIGURG
    lstate_num2tbl( L, "URG", SIGURG );
#endif
#ifdef SIGUSR1
    lstate_num2tbl( L, "USR1", SIGUSR1 );
#endif
#ifdef SIGUSR2
    lstate_num2tbl( L, "USR2", SIGUSR2 );
#endif
#ifdef SIGVTALRM
    lstate_num2tbl( L, "VTALRM", SIGVTALRM );
#endif
#ifdef SIGWINCH
    lstate_num2tbl( L, "WINCH", SIGWINCH );
#endif
#ifdef SIGXCPU
    lstate_num2tbl( L, "XCPU", SIGXCPU );
#endif
#ifdef SIGXFSZ
    lstate_num2tbl( L, "XFSZ", SIGXFSZ );
#endif

    
    return 1;
}
