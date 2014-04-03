var signames = {};

[
/*
    linux
    /usr/include/bits/signum.h
*/
'SIGHUP',
'SIGINT',
'SIGQUIT',
'SIGILL',
'SIGTRAP',
'SIGABRT',
'SIGIOT',
'SIGBUS',
'SIGFPE',
'SIGKILL',
'SIGUSR1',
'SIGSEGV',
'SIGUSR2',
'SIGPIPE',
'SIGALRM',
'SIGTERM',
'SIGSTKFLT',
'SIGCLD',
'SIGCHLD',
'SIGCONT',
'SIGSTOP',
'SIGTSTP',
'SIGTTIN',
'SIGTTOU',
'SIGURG',
'SIGXCPU',
'SIGXFSZ',
'SIGVTALRM',
'SIGPROF',
'SIGWINCH',
'SIGPOLL',
'SIGIO',
'SIGPWR',
'SIGSYS',
/*
    osx
    /include/sys/signal.h
*/
'SIGHUP',
'SIGINT',
'SIGQUIT',
'SIGILL',
'SIGTRAP',
'SIGABRT',
'SIGPOLL',
'SIGIOT',
'SIGEMT',
'SIGFPE',
'SIGKILL',
'SIGBUS',
'SIGSEGV',
'SIGSYS',
'SIGPIPE',
'SIGALRM',
'SIGTERM',
'SIGURG',
'SIGSTOP',
'SIGTSTP',
'SIGCONT',
'SIGCHLD',
'SIGTTIN',
'SIGTTOU',
'SIGIO',
'SIGXCPU',
'SIGXFSZ',
'SIGVTALRM',
'SIGPROF',
'SIGWINCH',
'SIGINFO',
'SIGUSR1',
'SIGUSR2']
.map(function(name){
    var shortName = name.replace( /^SIG/, '' );
    
    if( !signames[shortName] ){
        signames[shortName] = name;
    }
});

Object.keys( signames )
.sort()
.forEach(function(name){
    console.log( '#ifdef ' + signames[name] );
    console.log( 
        '    lstate_num2tbl( L, "%s", %s );'
        , name, signames[name]
    );
    console.log( '#endif' );
});
console.log( Object.keys( signames ).length );
